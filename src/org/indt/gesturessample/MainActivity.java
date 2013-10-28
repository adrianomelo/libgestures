package org.indt.gesturessample;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MotionEvent;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends Activity {

    private SurfaceViewRect surfaceView;
    public static native void nativeOnTouch(MotionEvent event);
    public static native void nativeOnStart(MainActivity activity);
    public static native void nativeOnStop();

    static {
        System.loadLibrary("gesturessample");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        // XXX: Avoid optimization
        setGestureType("No gesture");
        
        surfaceView = (SurfaceViewRect)findViewById(R.id.surfaceView);
        surfaceView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                nativeOnTouch(event);
                return true;
            }
        });
    }

    public void setGestureType(String gestureType) {
        TextView textView = (TextView)findViewById(R.id.text);
        textView.setText(gestureType);
    }
    
    public void updatePinch(double scale, int centerX, int centerY) {
        surfaceView.updatePinch(scale, centerX, centerY);
    }
    
    public void startPinch() {
        surfaceView.startPinch();
    }
    
    public void stopPinch() {
        surfaceView.stopPinch();
    }
    
    public void tap(int x, int y, int verticalRadius, int horizontalRadius) {
        surfaceView.tap(x, y, verticalRadius, horizontalRadius);
    }
    
    public void pan(int x, int y, int deltaX, int deltaY) {
        surfaceView.pan(x, y, deltaX, deltaY);
    }

    @Override
    protected void onStart() {
        super.onStart();
        nativeOnStart(this);
    }

    @Override
    protected void onStop() {
        super.onStop();
        nativeOnStop();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
}
