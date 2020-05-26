/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.xx;

import android.os.Bundle;
import android.view.Surface;

public class Activity extends android.app.Activity
{
    View mView;

    @Override protected void onCreate(Bundle icicle)
    {
        super.onCreate(icicle);
        mView = new View(getApplication());
        setContentView(mView);
    }

    @Override protected void onPause()
    {
        super.onPause();
        mView.onPause();
    }

    @Override protected void onResume()
    {
        super.onResume();
        mView.onResume();
    }

    // Used to load the 'native-lib' library on application startup.
    static
    {
        System.loadLibrary("xxImGui");
    }

    /**
     * A native method that is implemented by the 'xxImGui' native library,
     * which is packaged with this application.
     */
    public static native void create(Context context, Surface surface);
    public static native void resize(int width, int height, Surface surface);
    public static native void step(Context context);
    public static native void shutdown();
    public static native void pause();
    public static native void resume();
    public static native void touch(int type, float x, float y);
}
