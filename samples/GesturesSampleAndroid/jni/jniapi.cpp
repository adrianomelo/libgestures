#include <stdint.h>
#include <jni.h>
#include <android/input.h>

#include <gesturemanager.h>
#include <gestures/tap.h>
#include <gestures/pinch.h>
#include <gestures/pinch3f.h>
#include <gestures/pan.h>
#include <gestures/doubletap.h>
#include <gestures/longpress.h>
#include <events/ainputeventhelper.h>

#include "jniapi.h"
#include "logger.h"

#define LOG_TAG "GesturesSample-JNI"

static JavaVM *jvm = 0;
static jclass motionEventClass;
static jobject mainActivity;
static GestureManager* gestureManager;
static PanRecognizer *panRecognizer = 0;
static Gesture::GestureType currentGesture = Gesture::NoGesture;

void createGestureManager();
void updateGesture(Gesture *gesture);
void updateGestureType(Gesture::GestureType type);
void pinchUpdated(double, int, int);
void pinchStarted();
void pinchFinished();
void tap(int, int, int, int);
void pan(int, int, int, int);

JNIEnv* getEnv()
{
    LOG_INFO("getEnv()");

    JNIEnv *env = 0;
    jint rs = jvm->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (rs == JNI_EDETACHED) {
        LOG_INFO("EDETACHED");
        if (jvm->AttachCurrentThread(&env, NULL) < 0) {
            LOG_INFO("Failed to get the environment using AttachCurrentThread()");
        }
    } else if (rs == JNI_EVERSION) {
        LOG_INFO("Failed to get the environment. EVERSION error");
    }

    return env;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*)
{
    LOG_INFO("JNI_OnLoad");

    jvm = vm;
    JNIEnv *env = getEnv();

    jclass clazz = env->FindClass("android/view/MotionEvent");
    motionEventClass = reinterpret_cast<jclass>(env->NewGlobalRef(clazz));

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL Java_org_indt_gesturessample_MainActivity_nativeOnStart(JNIEnv* jenv, jobject, jobject activity)
{
    LOG_INFO("nativeOnStart");
    createGestureManager();
    mainActivity = jenv->NewGlobalRef(activity);
}

JNIEXPORT void JNICALL Java_org_indt_gesturessample_MainActivity_nativeOnStop(JNIEnv*, jobject)
{
    LOG_INFO("nativeOnStop");
    delete gestureManager;
}

JNIEXPORT void JNICALL Java_org_indt_gesturessample_MainActivity_nativeOnTouch(JNIEnv *env, jobject, jobject motionEvent)
{
    static jfieldID field = env->GetFieldID(motionEventClass, "mNativePtr", "I");
    jint nativeEvent = env->GetIntField(motionEvent, field);

    const AInputEvent* ev = (AInputEvent*)nativeEvent;
    if (!ev)
        return;

    if (AInputEvent_getType(ev) != AINPUT_EVENT_TYPE_MOTION)
        return;

    for (unsigned int i = 0; i < AMotionEvent_getPointerCount(ev); ++i) {
        LOG_INFO("id: %d; x: %f; y: %f", AMotionEvent_getPointerId(ev, i), AMotionEvent_getX(ev, i), AMotionEvent_getY(ev, i));
    }

    GestureTouchEvent touchEvent = convertToGestureTouchEvent(ev);
    Gesture *gesture = gestureManager->sendEvent(&touchEvent, 0);
    if (!gesture)
        updateGestureType(Gesture::NoGesture);
    else
        updateGesture(gesture);
}

JNIEXPORT void JNICALL Java_org_indt_gesturessample_MainActivity_nativeUpdateTimestamp(JNIEnv*, jobject, jlong timestamp)
{
    if (!gestureManager)
        return;

    Gesture *gesture = gestureManager->sendEvent(0, timestamp);
    updateGesture(gesture);
}

JNIEXPORT void JNICALL Java_org_indt_gesturessample_MainActivity_nativeSetSwipeArea(JNIEnv*, jobject, jint left, jint top, jint right, jint bottom)
{
    if (panRecognizer)
        panRecognizer->setBounds(left, top, right - left, bottom - top);
    else
        LOG_INFO("NO SWIPEREC :(");
}

void createGestureManager()
{
    gestureManager = new GestureManager();
    gestureManager->setMoveThreshold(40);

    // Create recognizers used in the app
    // Gesture manager takes ownership of the recognizers
    gestureManager->registerRecognizer(new TapRecognizer);
    panRecognizer = new PanRecognizer;
    gestureManager->registerRecognizer(panRecognizer);
    gestureManager->registerRecognizer(new PinchRecognizer);
    gestureManager->registerRecognizer(new Pinch3fRecognizer);
    gestureManager->registerRecognizer(new DoubleTapRecognizer);
    gestureManager->registerRecognizer(new LongPressRecognizer);
}

void updateGesture(Gesture *gesture)
{
    if (!gesture)
        return;

    updateGestureType(gesture->gestureType());

    if (gesture->gestureType() == Gesture::Pinch) {
        PinchGesture *pinch = static_cast<PinchGesture *>(gesture);
        if (pinch->gestureState() == Gesture::GestureStarted)
            pinchStarted();
        else if (pinch->gestureState() == Gesture::GestureUpdated)
            pinchUpdated(pinch->scale, pinch->centerX, pinch->centerY);
        else
            pinchFinished();
    } else if (gesture->gestureType() == Gesture::Pinch3f) {
        Pinch3fGesture *pinch = static_cast<Pinch3fGesture *>(gesture);
        if (pinch->gestureState() == Gesture::GestureStarted)
            pinchStarted();
        else if (pinch->gestureState() == Gesture::GestureUpdated)
            pinchUpdated(pinch->scale, pinch->centerX, pinch->centerY);
        else
            pinchFinished();
    } else if (gesture->gestureType() == Gesture::Tap) {
        TapGesture *tapGesture = static_cast<TapGesture *>(gesture);
        tap(tapGesture->x, tapGesture->y, tapGesture->verticalRadius, tapGesture->horizontalRadius);
    } else if (gesture->gestureType() == Gesture::Pan) {
        PanGesture *panGesture = static_cast<PanGesture *>(gesture);

        if (panGesture->side & (PanGesture::Left | PanGesture::Right))
            pan(panGesture->x, 0, panGesture->deltaX, 0);
        else if (panGesture->side & (PanGesture::Top | PanGesture::Bottom))
            pan(0, panGesture->y, 0, panGesture->deltaY);
        else
            pan(panGesture->x, panGesture->y, panGesture->deltaX, panGesture->deltaY);

        if (panGesture->side == PanGesture::Left)
            LOG_INFO("Swipe Left");
        else if (panGesture->side == PanGesture::Top)
            LOG_INFO("Swipe Top");
        else if (panGesture->side == PanGesture::Right)
            LOG_INFO("Swipe Right");
        else if (panGesture->side == PanGesture::Bottom)
            LOG_INFO("Swipe Bottom");
        else if (panGesture->side != PanGesture::NoSide)
            LOG_INFO("Swipe %d", panGesture->side);
    }
}

void updateGestureType(Gesture::GestureType type)
{
    if (type == currentGesture)
        return;

    currentGesture = type;

    jstring gestureTypeString;
    JNIEnv *env = getEnv();

    switch (currentGesture) {
        case Gesture::NoGesture:
            gestureTypeString = env->NewStringUTF("No gesture");
            break;

        case Gesture::Tap:
            gestureTypeString = env->NewStringUTF("Tap");
            break;

        case Gesture::Pinch:
            gestureTypeString = env->NewStringUTF("Pinch");
            break;

        case Gesture::Pinch3f:
            gestureTypeString = env->NewStringUTF("Pinch3f");
            break;

        case Gesture::Pan:
            gestureTypeString = env->NewStringUTF("Pan");
            break;

        case Gesture::DoubleTap:
            gestureTypeString = env->NewStringUTF("Double tap");
            break;

        case Gesture::LongPress:
            gestureTypeString = env->NewStringUTF("Long press");
            break;

        default:
            gestureTypeString = env->NewStringUTF("Unknown gesture");
            break;
    }

    jclass clazz = env->GetObjectClass(mainActivity);
    jmethodID methodSetGestureType = env->GetMethodID(clazz, "setGestureType", "(Ljava/lang/String;)V");
    if (methodSetGestureType)
        env->CallVoidMethod(mainActivity, methodSetGestureType, gestureTypeString);
}

void pinchUpdated(double scale, int centerX, int centerY)
{
    JNIEnv *env = getEnv();
    jclass clazz = env->GetObjectClass(mainActivity);
    jmethodID method = env->GetMethodID(clazz, "updatePinch", "(DII)V");
    if (method)
        env->CallVoidMethod(mainActivity, method, scale, centerX, centerY);
}

void pinchStarted()
{
    JNIEnv *env = getEnv();
    jclass clazz = env->GetObjectClass(mainActivity);
    jmethodID method = env->GetMethodID(clazz, "startPinch", "()V");
    if (method)
        env->CallVoidMethod(mainActivity, method);
}

void pinchFinished()
{
    JNIEnv *env = getEnv();
    jclass clazz = env->GetObjectClass(mainActivity);
    jmethodID method = env->GetMethodID(clazz, "stopPinch", "()V");
    if (method)
        env->CallVoidMethod(mainActivity, method);
}

void tap(int x, int y, int vertical, int horizontal)
{
    JNIEnv *env = getEnv();
    jclass clazz = env->GetObjectClass(mainActivity);
    jmethodID method = env->GetMethodID(clazz, "tap", "(IIII)V");
    if (method)
        env->CallVoidMethod(mainActivity, method, x, y, vertical, horizontal);
}

void pan(int x, int y, int deltaX, int deltaY)
{
    JNIEnv *env = getEnv();
    jclass clazz = env->GetObjectClass(mainActivity);
    jmethodID method = env->GetMethodID(clazz, "pan", "(IIII)V");
    if (method)
        env->CallVoidMethod(mainActivity, method, x, y, deltaX, deltaY);
}
