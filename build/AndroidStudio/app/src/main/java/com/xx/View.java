/*
 * Copyright (C) 2009 The Android Open Source Project
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

import android.content.Context;

class View extends SurfaceView
{
    private static boolean PAUSE = false;
    private Renderer RENDERER = new Renderer();

    public View(Context context)
    {
        super(context);

        /* Set the renderer responsible for frame rendering */
        setRenderer(RENDERER);
    }

    @Override
    public void onPause()
    {
        super.onPause();
        Activity.pause();
        PAUSE = true;
    }

    @Override
    public void onResume()
    {
        super.onResume();
        Activity.resume();
        PAUSE = false;
    }

    private class Renderer implements SurfaceView.Renderer
    {
        public void onDrawFrame()
        {
            if (PAUSE)
            {
                try
                {
                    Thread.sleep(1000);
                }
                catch(InterruptedException e)
                {
                }
                return;
            }
            Activity.step();
        }

        public void onSurfaceChanged(int width, int height)
        {
            Activity.resize(width, height, getHolder().getSurface());
        }

        public void onSurfaceCreated()
        {
            Activity.create(getHolder().getSurface());
        }
    }
}
