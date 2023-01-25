/*
 * Copyright (C) 2020, 2022 Max Kolesnikov <maxklvd@gmail.com>
 *
 * This file is part of LiteLockr.
 *
 * LiteLockr is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LiteLockr is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LiteLockr.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FLYOUT_RESOURCES_H
#define FLYOUT_RESOURCES_H

#include <array>
#include <vector>

#include "app/animation/AnimationFrameSet.h"
#include "gfx/Bitmap.h"
#include "ren/Geometry.h"
#include "ren/Material.h"
#include "ren/PlanesGeometry.h"
#include "ren/Renderer.h"

#include "res/Resources.h"

namespace litelockr {

struct FlyoutResources {
    struct Title {
        Bitmap background;
        Bitmap close;
        Bitmap menu;
    } title;

    struct Options {
        Bitmap settings;
        Bitmap help;
        Bitmap about;
    } menuBar;

    struct Body {
        Bitmap progressFull;

        struct Animations {
            Bitmap backLayers;
            AnimationFrameSet startLockingAnimation;
            AnimationFrameSet finishLockingAnimation;
            AnimationFrameSet cannotUnlockAnimation;
            AnimationFrameSet unlockAnimation;
        };
        Animations menuClosed;
        Animations menuOpen;
    } body;

    AnimationFrameSet hideFlyoutAnimation;
    AnimationFrameSet showFlyoutAnimation;

    AnimationFrameSet shakeAnimation;
    AnimationFrameSet startupAnimation;

    AnimationFrameSet openMenuAnimation;
    AnimationFrameSet closeMenuAnimation;

    AnimationFrameSet showMenuAnimation;
    AnimationFrameSet hideMenuAnimation;

    struct FlyoutScene {
        Bitmap renderBuffer;
        Renderer renderer{renderBuffer};
        SceneObjects objects;
        RECT viewFrame{};

        NullObject nullObj;
        MeshObject planes;
        PlanesGeometry planesGeo;
        Material planeLeftMat;
        Material planeRightMat;
        Material planeBodyMat{IDB_BODY};

        PlaneGeometry circleGeo;
        PlaneGeometry progressBackgroundGeo;
        PlaneGeometry progressGeo;
        PlaneGeometry lockGeo;
        PlaneGeometry keyboardGeo;
        PlaneGeometry mouseGeo;

        MeshObject circle;
        MeshObject progressBackground;
        MeshObject progress;
        MeshObject lock;
        MeshObject keyboard;
        MeshObject mouse;

        Material circleMat{IDB_CIRCLE};
        Material progressBackgroundMat{IDB_PROGRESS_BACKGROUND};
        Material progressMat{IDB_PROGRESS_FULL};

        Material lockOpenMat{IDB_LOCK_OPEN};
        Material lockClosedMat{IDB_LOCK_CLOSED};
        Material lockDisabledMat{IDB_LOCK_DISABLED};
        Material lockCancelMat{IDB_LOCK_CANCEL};
        Material lockBufferMat;

        Material keyboardEnabledMat{IDB_KEYBOARD_ENABLED};
        Material keyboardDisabledMat{IDB_KEYBOARD_DISABLED};
        Material mouseEnabledMat{IDB_MOUSE_ENABLED};
        Material mouseDisabledMat{IDB_MOUSE_DISABLED};
    } scene;

    enum KeyboardMaterialIndex {
        KEYBOARD_ENABLED_MAT = 0,
        KEYBOARD_DISABLED_MAT = 1,
    };

    enum MouseMaterialIndex {
        MOUSE_ENABLED_MAT = 0,
        MOUSE_DISABLED_MAT = 1,
    };

    enum LockMaterialIndex {
        LOCK_OPEN_MAT = 0,
        LOCK_CLOSED_MAT = 1,
        LOCK_CANCEL_MAT = 2,
        LOCK_DISABLED_MAT = 3,
        LOCK_BUFFER_MAT = 4,
    };

    using MenuAnimationValues = std::array<float, 12>;
    // pre-rendered values
    constexpr static MenuAnimationValues openMenuValues{
            0.0f,
            0.13599f,
            0.26972f,
            0.39895f,
            0.52141f,
            0.63486f,
            0.73704f,
            0.82570f,
            0.89857f,
            0.95342f,
            0.98798f,
            1.0f,
    };

    // pre-rendered values
    constexpr static MenuAnimationValues closeMenuValues{
            1.0f,
            0.97352f,
            0.90069f,
            0.79301f,
            0.66307f,
            0.52337f,
            0.38535f,
            0.25881f,
            0.15162f,
            0.06982f,
            0.01803f,
            0.0f,
    };
};

struct NumberOfFrames {
    constexpr static int STARTUP = 27;
    constexpr static int OPEN_MENU = 12;
    constexpr static int CLOSE_MENU = 12;
    constexpr static int START_LOCKING = 16;
    constexpr static int FINISH_LOCKING = 24;
    constexpr static int CANNOT_UNLOCK = 11;
    constexpr static int UNLOCK = 21;
};

} //namespace litelockr

#endif // FLYOUT_RESOURCES_H
