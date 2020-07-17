/*
 * Copyright (C) 2008 The Android Open Source Project
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

package android.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.SurfaceHolder;

import java.lang.ref.WeakReference;
import java.util.ArrayList;

/**
 * An implementation of SurfaceView that uses the dedicated surface for
 * displaying API rendering.
 */
public class NativeSurfaceView extends android.view.SurfaceView implements SurfaceHolder.Callback2
{
    private final static String TAG = "NativeSurfaceView";

    /**
     * Standard View constructor.
     */
    public NativeSurfaceView(Context context)
    {
        super(context);
        init();
    }

    /**
     * Standard View constructor.
     */
    public NativeSurfaceView(Context context, AttributeSet attrs)
    {
        super(context, attrs);
        init();
    }

    @Override
    protected void finalize() throws Throwable
    {
        try
        {
            if (mRenderThread != null)
            {
                // RenderThread may still be running if this view was never
                // attached to a window.
                mRenderThread.requestExitAndWait();
            }
        }
        finally
        {
            super.finalize();
        }
    }

    private void init()
    {
        // Install a SurfaceHolder.Callback so we get notified when the
        // underlying surface is created and destroyed
        SurfaceHolder holder = getHolder();
        holder.addCallback(this);
        // setFormat is done by SurfaceView in SDK 2.3 and newer. Uncomment
        // this statement if back-porting to 2.2 or older:
        // holder.setFormat(PixelFormat.RGB_565);
        //
        // setType is not needed for SDK 2.0 or newer. Uncomment this
        // statement if back-porting this code to older SDKs.
        // holder.setType(SurfaceHolder.SURFACE_TYPE_GPU);
    }

    /**
     * Set the renderer associated with this view.
     * @param renderer the renderer to use to perform API drawing.
     */
    public void setRenderer(Renderer renderer)
    {
        checkRenderThreadState();
        mRenderer = renderer;
        mRenderThread = new RenderThread(mThisWeakRef);
        mRenderThread.start();
    }

    /**
     * This method is part of the SurfaceHolder.Callback interface, and is
     * not normally called or subclassed by clients of NativeSurfaceView.
     */
    public void surfaceCreated(SurfaceHolder holder)
    {
        mRenderThread.surfaceCreated();
    }

    /**
     * This method is part of the SurfaceHolder.Callback interface, and is
     * not normally called or subclassed by clients of NativeSurfaceView.
     */
    public void surfaceDestroyed(SurfaceHolder holder)
    {
        // Surface will be destroyed when we return
        mRenderThread.surfaceDestroyed();
    }

    /**
     * This method is part of the SurfaceHolder.Callback interface, and is
     * not normally called or subclassed by clients of NativeSurfaceView.
     */
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
    {
        mRenderThread.onWindowResize(w, h);
    }

    /**
     * This method is part of the SurfaceHolder.Callback2 interface, and is
     * not normally called or subclassed by clients of NativeSurfaceView.
     */
    @Deprecated
    @Override
    public void surfaceRedrawNeeded(SurfaceHolder holder)
    {
        // Since we are part of the framework we know only surfaceRedrawNeededAsync
        // will be called.
    }

    /**
     * Pause the rendering thread.
     */
    public void onPause()
    {
        mRenderThread.onPause();
    }

    /**
     * Resumes the rendering thread.
     */
    public void onResume()
    {
        mRenderThread.onResume();
    }

    /**
     * Queue a runnable to be run on the API rendering thread.
     */
    public void queueEvent(Runnable r)
    {
        mRenderThread.queueEvent(r);
    }

    /**
     * This method is used as part of the View class and is not normally
     * called or subclassed by clients of NativeSurfaceView.
     */
    @Override
    protected void onAttachedToWindow()
    {
        super.onAttachedToWindow();
        if (mDetached && (mRenderer != null))
        {
            mRenderThread = new RenderThread(mThisWeakRef);
            mRenderThread.start();
        }
        mDetached = false;
    }

    @Override
    protected void onDetachedFromWindow()
    {
        if (mRenderThread != null)
        {
            mRenderThread.requestExitAndWait();
        }
        mDetached = true;
        super.onDetachedFromWindow();
    }

    /**
     * A generic renderer interface.
     */
    public interface Renderer
    {
        /**
         * Called when the surface is created or recreated.
         */
        void onSurfaceCreated();

        /**
         * Called when the surface changed size.
         * @param width
         * @param height
         */
        void onSurfaceChanged(int width, int height);

        /**
         * Called to draw the current frame.
         */
        void onDrawFrame();
    }

    /**
     * A generic API Thread.
     */
    static class RenderThread extends Thread
    {
        RenderThread(WeakReference<NativeSurfaceView> nativeSurfaceViewWeakRef)
        {
            super();
            mWidth = 0;
            mHeight = 0;
            mNativeSurfaceViewWeakRef = nativeSurfaceViewWeakRef;
        }

        @Override
        public void run()
        {
            setName("RenderThread " + getId());
            try
            {
                guardedRun();
            }
            catch (InterruptedException e)
            {
                // fall thru and exit normally
            }
            finally
            {
                sRenderThreadManager.threadExiting(this);
            }
        }

        private void guardedRun() throws InterruptedException
        {
            mHaveAPISurface = false;

            try
            {
                boolean createAPISurface = false;
                boolean sizeChanged = false;
                boolean wantRenderNotification = false;
                boolean doRenderNotification = false;
                int w = 0;
                int h = 0;
                Runnable event = null;

                while (true)
                {
                    synchronized (sRenderThreadManager)
                    {
                        while (true)
                        {
                            if (mShouldExit)
                            {
                                return;
                            }

                            if (!mEventQueue.isEmpty())
                            {
                                event = mEventQueue.remove(0);
                                break;
                            }

                            // Update the pause state.
                            boolean pausing = false;
                            if (mPaused != mRequestPaused)
                            {
                                pausing = mRequestPaused;
                                mPaused = mRequestPaused;
                                sRenderThreadManager.notifyAll();
                            }

                            // When pausing, release the API surface:
                            if (pausing && mHaveAPISurface)
                            {
                                mHaveAPISurface = false;
                            }

                            // Have we lost the SurfaceView surface?
                            if ((!mHasSurface) && (!mWaitingForSurface))
                            {
                                mHaveAPISurface = false;
                                mWaitingForSurface = true;
                                sRenderThreadManager.notifyAll();
                            }

                            // Have we acquired the surface view surface?
                            if (mHasSurface && mWaitingForSurface)
                            {
                                mWaitingForSurface = false;
                                sRenderThreadManager.notifyAll();
                            }

                            if (doRenderNotification)
                            {
                                wantRenderNotification = false;
                                doRenderNotification = false;
                                mRenderComplete = true;
                                sRenderThreadManager.notifyAll();
                            }

                            // Ready to draw?
                            if (readyToDraw())
                            {
                                if (!mHaveAPISurface)
                                {
                                    mHaveAPISurface = true;
                                    createAPISurface = true;
                                    sizeChanged = true;
                                }

                                if (mHaveAPISurface)
                                {
                                    if (mSizeChanged)
                                    {
                                        sizeChanged = true;
                                        w = mWidth;
                                        h = mHeight;
                                        wantRenderNotification = true;

                                        // Destroy and recreate the API surface.
                                        createAPISurface = true;

                                        mSizeChanged = false;
                                    }
                                    sRenderThreadManager.notifyAll();
                                    break;
                                }
                            }

                            // By design, this is the only place in a RenderThread thread where we wait().
                            sRenderThreadManager.wait();
                        }
                    } // end of synchronized (sRenderThreadManager)

                    if (event != null)
                    {
                        event.run();
                        event = null;
                        continue;
                    }

                    if (createAPISurface)
                    {
                        NativeSurfaceView view = mNativeSurfaceViewWeakRef.get();
                        if (view != null)
                        {
                            view.mRenderer.onSurfaceCreated();
                        }
                        synchronized (sRenderThreadManager)
                        {
                            mFinishedCreatingAPISurface = true;
                            sRenderThreadManager.notifyAll();
                        }
                        createAPISurface = false;
                    }

                    if (sizeChanged)
                    {
                        NativeSurfaceView view = mNativeSurfaceViewWeakRef.get();
                        if (view != null)
                        {
                            view.mRenderer.onSurfaceChanged(w, h);
                        }
                        sizeChanged = false;
                    }

                    NativeSurfaceView view = mNativeSurfaceViewWeakRef.get();
                    if (view != null)
                    {
                        view.mRenderer.onDrawFrame();
                    }

                    if (wantRenderNotification)
                    {
                        doRenderNotification = true;
                        wantRenderNotification = false;
                    }
                }
            }
            finally
            {
                /*
                 * clean-up everything...
                 */
                synchronized (sRenderThreadManager)
                {
                    mHaveAPISurface = false;
                }
            }
        }

        public boolean ableToDraw()
        {
            return mHaveAPISurface && readyToDraw();
        }

        private boolean readyToDraw()
        {
            return (!mPaused) && mHasSurface && (mWidth > 0) && (mHeight > 0);
        }

        public void surfaceCreated()
        {
            synchronized (sRenderThreadManager)
            {
                mHasSurface = true;
                mFinishedCreatingAPISurface = false;
                sRenderThreadManager.notifyAll();
                while (mWaitingForSurface && (!mFinishedCreatingAPISurface) && (!mExited))
                {
                    try
                    {
                        sRenderThreadManager.wait();
                    }
                    catch (InterruptedException e)
                    {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }

        public void surfaceDestroyed()
        {
            synchronized (sRenderThreadManager)
            {
                mHasSurface = false;
                sRenderThreadManager.notifyAll();
                while ((!mWaitingForSurface) && (!mExited))
                {
                    try
                    {
                        sRenderThreadManager.wait();
                    }
                    catch (InterruptedException e)
                    {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }

        public void onPause()
        {
            synchronized (sRenderThreadManager)
            {
                mRequestPaused = true;
                sRenderThreadManager.notifyAll();
                while ((!mExited) && (!mPaused))
                {
                    try
                    {
                        sRenderThreadManager.wait();
                    }
                    catch (InterruptedException e)
                    {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }

        public void onResume()
        {
            synchronized (sRenderThreadManager)
            {
                mRequestPaused = false;
                mRenderComplete = false;
                sRenderThreadManager.notifyAll();
                while ((!mExited) && mPaused && (!mRenderComplete))
                {
                    try
                    {
                        sRenderThreadManager.wait();
                    }
                    catch (InterruptedException e)
                    {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }

        public void onWindowResize(int w, int h)
        {
            synchronized (sRenderThreadManager)
            {
                mWidth = w;
                mHeight = h;
                mSizeChanged = true;
                mRenderComplete = false;

                // If we are already on the API thread, this means a client callback
                // has caused reentrancy, for example via updating the SurfaceView parameters.
                // We need to process the size change eventually though and update our Surface.
                // So we set the parameters and return so they can be processed on our
                // next iteration.
                if (Thread.currentThread() == this)
                {
                    return;
                }

                sRenderThreadManager.notifyAll();

                // Wait for thread to react to resize and render a frame
                while ((!mExited) && (!mPaused) && (!mRenderComplete) && ableToDraw())
                {
                    try
                    {
                        sRenderThreadManager.wait();
                    }
                    catch (InterruptedException e)
                    {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }

        public void requestExitAndWait()
        {
            // don't call this from RenderThread thread or it is a guaranteed
            // deadlock!
            synchronized (sRenderThreadManager)
            {
                mShouldExit = true;
                sRenderThreadManager.notifyAll();
                while (!mExited)
                {
                    try
                    {
                        sRenderThreadManager.wait();
                    }
                    catch (InterruptedException e)
                    {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }

        /**
         * Queue an "event" to be run on the API rendering thread.
         * @param r the runnable to be run on the API rendering thread.
         */
        public void queueEvent(Runnable r)
        {
            if (r == null)
            {
                throw new IllegalArgumentException("r must not be null");
            }
            synchronized (sRenderThreadManager)
            {
                mEventQueue.add(r);
                sRenderThreadManager.notifyAll();
            }
        }

        // Once the thread is started, all accesses to the following member
        // variables are protected by the sRenderThreadManager monitor
        private boolean mShouldExit;
        private boolean mExited;
        private boolean mRequestPaused;
        private boolean mPaused;
        private boolean mHasSurface;
        private boolean mWaitingForSurface;
        private boolean mHaveAPISurface;
        private boolean mFinishedCreatingAPISurface;
        private int mWidth;
        private int mHeight;
        private boolean mRenderComplete;
        private ArrayList<Runnable> mEventQueue = new ArrayList<Runnable>();
        private boolean mSizeChanged = true;

        // End of member variables protected by the sRenderThreadManager monitor.

        /**
         * Set once at thread construction time, nulled out when the parent view is garbage
         * called. This weak reference allows the SurfaceView to be garbage collected while
         * the RenderThread is still alive.
         */
        private WeakReference<NativeSurfaceView> mNativeSurfaceViewWeakRef;
    }

    private void checkRenderThreadState()
    {
        if (mRenderThread != null)
        {
            throw new IllegalStateException("setRenderer has already been called for this instance.");
        }
    }

    private static class RenderThreadManager
    {
        private static String TAG = "RenderThreadManager";

        public synchronized void threadExiting(RenderThread thread)
        {
            thread.mExited = true;
            notifyAll();
        }
    }

    private static final RenderThreadManager sRenderThreadManager = new RenderThreadManager();

    private final WeakReference<NativeSurfaceView> mThisWeakRef = new WeakReference<NativeSurfaceView>(this);
    private RenderThread mRenderThread;
    private Renderer mRenderer;
    private boolean mDetached;
}
