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
import android.util.Log;

import java.lang.ref.WeakReference;
import java.util.ArrayList;

/**
 * An implementation of SurfaceView that uses the dedicated surface for
 * displaying API rendering.
 */
public class NativeSurfaceView extends SurfaceView implements SurfaceHolder.Callback2 {
    private final static String TAG = "NativeSurfaceView";
    private final static boolean LOG_ATTACH_DETACH = false;
    private final static boolean LOG_THREADS = false;
    private final static boolean LOG_PAUSE_RESUME = false;
    private final static boolean LOG_SURFACE = false;
    private final static boolean LOG_RENDERER = false;
    private final static boolean LOG_RENDERER_DRAW_FRAME = false;
    /**
     * The renderer only renders
     * when the surface is created, or when {@link #requestRender} is called.
     *
     * @see #getRenderMode()
     * @see #setRenderMode(int)
     * @see #requestRender()
     */
    public final static int RENDERMODE_WHEN_DIRTY = 0;
    /**
     * The renderer is called
     * continuously to re-render the scene.
     *
     * @see #getRenderMode()
     * @see #setRenderMode(int)
     */
    public final static int RENDERMODE_CONTINUOUSLY = 1;

    /**
     * Standard View constructor. In order to render something, you
     * must call {@link #setRenderer} to register a renderer.
     */
    public NativeSurfaceView(Context context) {
        super(context);
        init();
    }

    /**
     * Standard View constructor. In order to render something, you
     * must call {@link #setRenderer} to register a renderer.
     */
    public NativeSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            if (mRenderThread != null) {
                // RenderThread may still be running if this view was never
                // attached to a window.
                mRenderThread.requestExitAndWait();
            }
        } finally {
            super.finalize();
        }
    }

    private void init() {
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
     * Set the renderer associated with this view. Also starts the thread that
     * will call the renderer, which in turn causes the rendering to start.
     * @param renderer the renderer to use to perform API drawing.
     */
    public void setRenderer(Renderer renderer) {
        checkRenderThreadState();
        mRenderer = renderer;
        mRenderThread = new RenderThread(mThisWeakRef);
        mRenderThread.start();
    }

    /**
     * Set the rendering mode. When renderMode is
     * RENDERMODE_CONTINUOUSLY, the renderer is called
     * repeatedly to re-render the scene. When renderMode
     * is RENDERMODE_WHEN_DIRTY, the renderer only rendered when the surface
     * is created, or when {@link #requestRender} is called. Defaults to RENDERMODE_CONTINUOUSLY.
     * <p>
     * Using RENDERMODE_WHEN_DIRTY can improve battery life and overall system performance
     * by allowing the GPU and CPU to idle when the view does not need to be updated.
     * <p>
     * This method can only be called after {@link #setRenderer(Renderer)}
     *
     * @param renderMode one of the RENDERMODE_X constants
     * @see #RENDERMODE_CONTINUOUSLY
     * @see #RENDERMODE_WHEN_DIRTY
     */
    public void setRenderMode(int renderMode) {
        mRenderThread.setRenderMode(renderMode);
    }

    /**
     * Get the current rendering mode. May be called
     * from any thread. Must not be called before a renderer has been set.
     * @return the current rendering mode.
     * @see #RENDERMODE_CONTINUOUSLY
     * @see #RENDERMODE_WHEN_DIRTY
     */
    public int getRenderMode() {
        return mRenderThread.getRenderMode();
    }

    /**
     * Request that the renderer render a frame.
     * This method is typically used when the render mode has been set to
     * {@link #RENDERMODE_WHEN_DIRTY}, so that frames are only rendered on demand.
     * May be called
     * from any thread. Must not be called before a renderer has been set.
     */
    public void requestRender() {
        mRenderThread.requestRender();
    }

    /**
     * This method is part of the SurfaceHolder.Callback interface, and is
     * not normally called or subclassed by clients of NativeSurfaceView.
     */
    public void surfaceCreated(SurfaceHolder holder) {
        mRenderThread.surfaceCreated();
    }

    /**
     * This method is part of the SurfaceHolder.Callback interface, and is
     * not normally called or subclassed by clients of NativeSurfaceView.
     */
    public void surfaceDestroyed(SurfaceHolder holder) {
        // Surface will be destroyed when we return
        mRenderThread.surfaceDestroyed();
    }

    /**
     * This method is part of the SurfaceHolder.Callback interface, and is
     * not normally called or subclassed by clients of NativeSurfaceView.
     */
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        mRenderThread.onWindowResize(w, h);
    }

    /**
     * This method is part of the SurfaceHolder.Callback2 interface, and is
     * not normally called or subclassed by clients of NativeSurfaceView.
     */
    @Override
    public void surfaceRedrawNeededAsync(SurfaceHolder holder, Runnable finishDrawing) {
        if (mRenderThread != null) {
            mRenderThread.requestRenderAndNotify(finishDrawing);
        }
    }

    /**
     * This method is part of the SurfaceHolder.Callback2 interface, and is
     * not normally called or subclassed by clients of NativeSurfaceView.
     */
    @Deprecated
    @Override
    public void surfaceRedrawNeeded(SurfaceHolder holder) {
        // Since we are part of the framework we know only surfaceRedrawNeededAsync
        // will be called.
    }


    /**
     * Pause the rendering thread.
     *
     * This method should be called when it is no longer desirable for the
     * NativeSurfaceView to continue rendering, such as in response to
     * {@link android.app.Activity#onStop Activity.onStop}.
     *
     * Must not be called before a renderer has been set.
     */
    public void onPause() {
        mRenderThread.onPause();
    }

    /**
     * Resumes the rendering thread.
     *
     * This method should typically be called in
     * {@link android.app.Activity#onStart Activity.onStart}.
     *
     * Must not be called before a renderer has been set.
     */
    public void onResume() {
        mRenderThread.onResume();
    }

    /**
     * Queue a runnable to be run on the API rendering thread. This can be used
     * to communicate with the Renderer on the rendering thread.
     * Must not be called before a renderer has been set.
     * @param r the runnable to be run on the API rendering thread.
     */
    public void queueEvent(Runnable r) {
        mRenderThread.queueEvent(r);
    }

    /**
     * This method is used as part of the View class and is not normally
     * called or subclassed by clients of NativeSurfaceView.
     */
    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();
        if (LOG_ATTACH_DETACH) {
            Log.d(TAG, "onAttachedToWindow reattach =" + mDetached);
        }
        if (mDetached && (mRenderer != null)) {
            int renderMode = RENDERMODE_CONTINUOUSLY;
            if (mRenderThread != null) {
                renderMode = mRenderThread.getRenderMode();
            }
            mRenderThread = new RenderThread(mThisWeakRef);
            if (renderMode != RENDERMODE_CONTINUOUSLY) {
                mRenderThread.setRenderMode(renderMode);
            }
            mRenderThread.start();
        }
        mDetached = false;
    }

    @Override
    protected void onDetachedFromWindow() {
        if (LOG_ATTACH_DETACH) {
            Log.d(TAG, "onDetachedFromWindow");
        }
        if (mRenderThread != null) {
            mRenderThread.requestExitAndWait();
        }
        mDetached = true;
        super.onDetachedFromWindow();
    }

    /**
     * A generic renderer interface.
     *
     * @see #setRenderer(Renderer)
     */
    public interface Renderer {
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
     * A generic API Thread. Takes care of initializing API. Delegates
     * to a Renderer instance to do the actual drawing. Can be configured to
     * render continuously or on request.
     *
     * All potentially blocking synchronization is done through the
     * sRenderThreadManager object. This avoids multiple-lock ordering issues.
     *
     */
    static class RenderThread extends Thread {
        RenderThread(WeakReference<NativeSurfaceView> nativeSurfaceViewWeakRef) {
            super();
            mWidth = 0;
            mHeight = 0;
            mRequestRender = true;
            mRenderMode = RENDERMODE_CONTINUOUSLY;
            mWantRenderNotification = false;
            mNativeSurfaceViewWeakRef = nativeSurfaceViewWeakRef;
        }

        @Override
        public void run() {
            setName("RenderThread " + getId());
            if (LOG_THREADS) {
                Log.i("RenderThread", "starting tid=" + getId());
            }

            try {
                guardedRun();
            } catch (InterruptedException e) {
                // fall thru and exit normally
            } finally {
                sRenderThreadManager.threadExiting(this);
            }
        }

        private void guardedRun() throws InterruptedException {
            mHaveAPISurface = false;
            mWantRenderNotification = false;

            try {
                boolean createAPISurface = false;
                boolean sizeChanged = false;
                boolean wantRenderNotification = false;
                boolean doRenderNotification = false;
                int w = 0;
                int h = 0;
                Runnable event = null;
                Runnable finishDrawingRunnable = null;

                while (true) {
                    synchronized (sRenderThreadManager) {
                        while (true) {
                            if (mShouldExit) {
                                return;
                            }

                            if (! mEventQueue.isEmpty()) {
                                event = mEventQueue.remove(0);
                                break;
                            }

                            // Update the pause state.
                            boolean pausing = false;
                            if (mPaused != mRequestPaused) {
                                pausing = mRequestPaused;
                                mPaused = mRequestPaused;
                                sRenderThreadManager.notifyAll();
                                if (LOG_PAUSE_RESUME) {
                                    Log.i("RenderThread", "mPaused is now " + mPaused + " tid=" + getId());
                                }
                            }

                            // When pausing, release the API surface:
                            if (pausing && mHaveAPISurface) {
                                if (LOG_SURFACE) {
                                    Log.i("RenderThread", "releasing API surface because paused tid=" + getId());
                                }
                                mHaveAPISurface = false;
                            }

                            // Have we lost the SurfaceView surface?
                            if ((! mHasSurface) && (! mWaitingForSurface)) {
                                if (LOG_SURFACE) {
                                    Log.i("RenderThread", "noticed surfaceView surface lost tid=" + getId());
                                }
                                mHaveAPISurface = false;
                                mWaitingForSurface = true;
                                mSurfaceIsBad = false;
                                sRenderThreadManager.notifyAll();
                            }

                            // Have we acquired the surface view surface?
                            if (mHasSurface && mWaitingForSurface) {
                                if (LOG_SURFACE) {
                                    Log.i("RenderThread", "noticed surfaceView surface acquired tid=" + getId());
                                }
                                mWaitingForSurface = false;
                                sRenderThreadManager.notifyAll();
                            }

                            if (doRenderNotification) {
                                if (LOG_SURFACE) {
                                    Log.i("RenderThread", "sending render notification tid=" + getId());
                                }
                                mWantRenderNotification = false;
                                doRenderNotification = false;
                                mRenderComplete = true;
                                sRenderThreadManager.notifyAll();
                            }

                            if (mFinishDrawingRunnable != null) {
                                finishDrawingRunnable = mFinishDrawingRunnable;
                                mFinishDrawingRunnable = null;
                            }

                            // Ready to draw?
                            if (readyToDraw()) {
                                if (!mHaveAPISurface) {
                                    mHaveAPISurface = true;
                                    createAPISurface = true;
                                    sizeChanged = true;
                                }

                                if (mHaveAPISurface) {
                                    if (mSizeChanged) {
                                        sizeChanged = true;
                                        w = mWidth;
                                        h = mHeight;
                                        mWantRenderNotification = true;
                                        if (LOG_SURFACE) {
                                            Log.i("RenderThread",
                                                    "noticing that we want render notification tid="
                                                    + getId());
                                        }

                                        // Destroy and recreate the API surface.
                                        createAPISurface = true;

                                        mSizeChanged = false;
                                    }
                                    mRequestRender = false;
                                    sRenderThreadManager.notifyAll();
                                    if (mWantRenderNotification) {
                                        wantRenderNotification = true;
                                    }
                                    break;
                                }
                            } else {
                                if (finishDrawingRunnable != null) {
                                    Log.w(TAG, "Warning, !readyToDraw() but waiting for " +
                                            "draw finished! Early reporting draw finished.");
                                    finishDrawingRunnable.run();
                                    finishDrawingRunnable = null;
                                }
                            }
                            // By design, this is the only place in a RenderThread thread where we wait().
                            if (LOG_THREADS) {
                                Log.i("RenderThread", "waiting tid=" + getId()
                                    + " mHaveAPISurface: " + mHaveAPISurface
                                    + " mFinishedCreatingAPISurface: " + mFinishedCreatingAPISurface
                                    + " mPaused: " + mPaused
                                    + " mHasSurface: " + mHasSurface
                                    + " mSurfaceIsBad: " + mSurfaceIsBad
                                    + " mWaitingForSurface: " + mWaitingForSurface
                                    + " mWidth: " + mWidth
                                    + " mHeight: " + mHeight
                                    + " mRequestRender: " + mRequestRender
                                    + " mRenderMode: " + mRenderMode);
                            }
                            sRenderThreadManager.wait();
                        }
                    } // end of synchronized(sRenderThreadManager)

                    if (event != null) {
                        event.run();
                        event = null;
                        continue;
                    }

                    if (createAPISurface) {
                        if (LOG_SURFACE) {
                            Log.w("RenderThread", "API createSurface");
                        }
                        if (LOG_RENDERER) {
                            Log.w("RenderThread", "onSurfaceCreated");
                        }
                        NativeSurfaceView view = mNativeSurfaceViewWeakRef.get();
                        if (view != null) {
                            view.mRenderer.onSurfaceCreated();
                        }
                        synchronized (sRenderThreadManager) {
                            mFinishedCreatingAPISurface = true;
                            sRenderThreadManager.notifyAll();
                        }
                        createAPISurface = false;
                    }

                    if (sizeChanged) {
                        if (LOG_RENDERER) {
                            Log.w("RenderThread", "onSurfaceChanged(" + w + ", " + h + ")");
                        }
                        NativeSurfaceView view = mNativeSurfaceViewWeakRef.get();
                        if (view != null) {
                            view.mRenderer.onSurfaceChanged(w, h);
                        }
                        sizeChanged = false;
                    }

                    if (LOG_RENDERER_DRAW_FRAME) {
                        Log.w("RenderThread", "onDrawFrame tid=" + getId());
                    }
                    {
                        NativeSurfaceView view = mNativeSurfaceViewWeakRef.get();
                        if (view != null) {
                            view.mRenderer.onDrawFrame();
                            if (finishDrawingRunnable != null) {
                                finishDrawingRunnable.run();
                                finishDrawingRunnable = null;
                            }
                        }
                    }

                    if (wantRenderNotification) {
                        doRenderNotification = true;
                        wantRenderNotification = false;
                    }
                }

            } finally {
                /*
                 * clean-up everything...
                 */
                synchronized (sRenderThreadManager) {
                    mHaveAPISurface = false;
                }
            }
        }

        public boolean ableToDraw() {
            return mHaveAPISurface && readyToDraw();
        }

        private boolean readyToDraw() {
            return (!mPaused) && mHasSurface && (!mSurfaceIsBad)
                && (mWidth > 0) && (mHeight > 0)
                && (mRequestRender || (mRenderMode == RENDERMODE_CONTINUOUSLY));
        }

        public void setRenderMode(int renderMode) {
            if ( !((RENDERMODE_WHEN_DIRTY <= renderMode) && (renderMode <= RENDERMODE_CONTINUOUSLY)) ) {
                throw new IllegalArgumentException("renderMode");
            }
            synchronized(sRenderThreadManager) {
                mRenderMode = renderMode;
                sRenderThreadManager.notifyAll();
            }
        }

        public int getRenderMode() {
            synchronized(sRenderThreadManager) {
                return mRenderMode;
            }
        }

        public void requestRender() {
            synchronized(sRenderThreadManager) {
                mRequestRender = true;
                sRenderThreadManager.notifyAll();
            }
        }

        public void requestRenderAndNotify(Runnable finishDrawing) {
            synchronized(sRenderThreadManager) {
                // If we are already on the API thread, this means a client callback
                // has caused reentrancy, for example via updating the SurfaceView parameters.
                // We will return to the client rendering code, so here we don't need to
                // do anything.
                if (Thread.currentThread() == this) {
                    return;
                }

                mWantRenderNotification = true;
                mRequestRender = true;
                mRenderComplete = false;
                mFinishDrawingRunnable = finishDrawing;

                sRenderThreadManager.notifyAll();
            }
        }

        public void surfaceCreated() {
            synchronized(sRenderThreadManager) {
                if (LOG_THREADS) {
                    Log.i("RenderThread", "surfaceCreated tid=" + getId());
                }
                mHasSurface = true;
                mFinishedCreatingAPISurface = false;
                sRenderThreadManager.notifyAll();
                while (mWaitingForSurface
                       && !mFinishedCreatingAPISurface
                       && !mExited) {
                    try {
                        sRenderThreadManager.wait();
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }

        public void surfaceDestroyed() {
            synchronized(sRenderThreadManager) {
                if (LOG_THREADS) {
                    Log.i("RenderThread", "surfaceDestroyed tid=" + getId());
                }
                mHasSurface = false;
                sRenderThreadManager.notifyAll();
                while((!mWaitingForSurface) && (!mExited)) {
                    try {
                        sRenderThreadManager.wait();
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }

        public void onPause() {
            synchronized (sRenderThreadManager) {
                if (LOG_PAUSE_RESUME) {
                    Log.i("RenderThread", "onPause tid=" + getId());
                }
                mRequestPaused = true;
                sRenderThreadManager.notifyAll();
                while ((! mExited) && (! mPaused)) {
                    if (LOG_PAUSE_RESUME) {
                        Log.i("Main thread", "onPause waiting for mPaused.");
                    }
                    try {
                        sRenderThreadManager.wait();
                    } catch (InterruptedException ex) {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }

        public void onResume() {
            synchronized (sRenderThreadManager) {
                if (LOG_PAUSE_RESUME) {
                    Log.i("RenderThread", "onResume tid=" + getId());
                }
                mRequestPaused = false;
                mRequestRender = true;
                mRenderComplete = false;
                sRenderThreadManager.notifyAll();
                while ((! mExited) && mPaused && (!mRenderComplete)) {
                    if (LOG_PAUSE_RESUME) {
                        Log.i("Main thread", "onResume waiting for !mPaused.");
                    }
                    try {
                        sRenderThreadManager.wait();
                    } catch (InterruptedException ex) {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }

        public void onWindowResize(int w, int h) {
            synchronized (sRenderThreadManager) {
                mWidth = w;
                mHeight = h;
                mSizeChanged = true;
                mRequestRender = true;
                mRenderComplete = false;

                // If we are already on the API thread, this means a client callback
                // has caused reentrancy, for example via updating the SurfaceView parameters.
                // We need to process the size change eventually though and update our Surface.
                // So we set the parameters and return so they can be processed on our
                // next iteration.
                if (Thread.currentThread() == this) {
                    return;
                }

                sRenderThreadManager.notifyAll();

                // Wait for thread to react to resize and render a frame
                while (! mExited && !mPaused && !mRenderComplete
                        && ableToDraw()) {
                    if (LOG_SURFACE) {
                        Log.i("Main thread", "onWindowResize waiting for render complete from tid=" + getId());
                    }
                    try {
                        sRenderThreadManager.wait();
                    } catch (InterruptedException ex) {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }

        public void requestExitAndWait() {
            // don't call this from RenderThread thread or it is a guaranteed
            // deadlock!
            synchronized(sRenderThreadManager) {
                mShouldExit = true;
                sRenderThreadManager.notifyAll();
                while (! mExited) {
                    try {
                        sRenderThreadManager.wait();
                    } catch (InterruptedException ex) {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }

        /**
         * Queue an "event" to be run on the API rendering thread.
         * @param r the runnable to be run on the API rendering thread.
         */
        public void queueEvent(Runnable r) {
            if (r == null) {
                throw new IllegalArgumentException("r must not be null");
            }
            synchronized(sRenderThreadManager) {
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
        private boolean mSurfaceIsBad;
        private boolean mWaitingForSurface;
        private boolean mHaveAPISurface;
        private boolean mFinishedCreatingAPISurface;
        private int mWidth;
        private int mHeight;
        private int mRenderMode;
        private boolean mRequestRender;
        private boolean mWantRenderNotification;
        private boolean mRenderComplete;
        private ArrayList<Runnable> mEventQueue = new ArrayList<Runnable>();
        private boolean mSizeChanged = true;
        private Runnable mFinishDrawingRunnable = null;

        // End of member variables protected by the sRenderThreadManager monitor.

        /**
         * Set once at thread construction time, nulled out when the parent view is garbage
         * called. This weak reference allows the NativeSurfaceView to be garbage collected while
         * the RenderThread is still alive.
         */
        private WeakReference<NativeSurfaceView> mNativeSurfaceViewWeakRef;

    }

    private void checkRenderThreadState() {
        if (mRenderThread != null) {
            throw new IllegalStateException(
                    "setRenderer has already been called for this instance.");
        }
    }

    private static class RenderThreadManager {
        private static String TAG = "RenderThreadManager";

        public synchronized void threadExiting(RenderThread thread) {
            if (LOG_THREADS) {
                Log.i("RenderThread", "exiting tid=" +  thread.getId());
            }
            thread.mExited = true;
            notifyAll();
        }
    }

    private static final RenderThreadManager sRenderThreadManager = new RenderThreadManager();

    private final WeakReference<NativeSurfaceView> mThisWeakRef =
            new WeakReference<NativeSurfaceView>(this);
    private RenderThread mRenderThread;
    private Renderer mRenderer;
    private boolean mDetached;
}
