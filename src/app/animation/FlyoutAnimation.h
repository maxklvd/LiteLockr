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

#ifndef FLYOUT_ANIMATION_H
#define FLYOUT_ANIMATION_H

#include "AnimationFrameSet.h"
#include "AnimationHandler.h"
#include "app/FlyoutLayers.h"
#include "app/FlyoutModel.h"
#include "app/FlyoutResources.h"
#include "app/LockState.h"
#include "app/Sounds.h"
#include "TransitionEffects.h"
#include "ren/Material.h"
#include "sys/Rectangle.h"

#include "res/Resources.h"

namespace litelockr {

template<typename View>
class FlyoutAnimation {
public:
    FlyoutAnimation() = default;

    View& view() {
        return static_cast<View&>(*this);
    }

    void initializeAnimations() {
        auto& res = view().resources_;
        const auto& model = view().model_;

        // startup animation
        res.startupAnimation.loadPng(IDB_ANIM_STARTUP, NumberOfFrames::STARTUP);
        res.startupAnimation.setId(AnimationId::STARTUP);
        res.startupAnimation.setPosition(FlyoutModel::MARGIN, FlyoutModel::MARGIN);

        res.openMenuAnimation.loadPng(IDB_ANIM_OPEN_MENU, NumberOfFrames::OPEN_MENU);

        res.closeMenuAnimation.loadPng(IDB_ANIM_CLOSE_MENU, NumberOfFrames::CLOSE_MENU);

        auto& backMC = res.body.menuClosed.backLayers;
        auto& backMO = res.body.menuOpen.backLayers;
        backMC.createAs(view().buffer_);
        backMO.createAs(view().buffer_);

        view().renderBackground(0, FlyoutLayers::background(), false);
        view().drawBackground(backMC);

        view().renderBackground(1, FlyoutLayers::background(), false);
        view().drawBackground(backMO);

        {
            //
            //get background
            //
            Bitmap bg;
            bg.create(model.lock.w, model.lock.h);
            unsigned w = bg.width();
            unsigned h = bg.height();
            bg.copyFrom(backMC, 0, 0, model.lock.x, model.lock.y, w, h);

            {
                AnimationFrameSet startLockingAnimation;
                startLockingAnimation.loadPng(IDB_ANIM_START_LOCKING, NumberOfFrames::START_LOCKING);

                auto& animMC = res.body.menuClosed.startLockingAnimation;
                animMC.createWithBackground(startLockingAnimation.bitmap(), startLockingAnimation.size(), bg);
                animMC.setPosition(model.lock.x, model.lock.y);
                animMC.setId(AnimationId::START_LOCKING);

                auto& animMO = res.body.menuOpen.startLockingAnimation;
                createLockAnimation(startLockingAnimation, animMO);
                animMO.setId(AnimationId::START_LOCKING);
            }

            {
                AnimationFrameSet finishLockingAnimation;
                finishLockingAnimation.loadPng(IDB_ANIM_FINISH_LOCKING, NumberOfFrames::FINISH_LOCKING);

                auto& animMC = res.body.menuClosed.finishLockingAnimation;
                animMC.createWithBackground(finishLockingAnimation.bitmap(), finishLockingAnimation.size(), bg);
                animMC.setPosition(model.lock.x, model.lock.y);
                animMC.setId(AnimationId::FINISH_LOCKING);

                auto& animMO = res.body.menuOpen.finishLockingAnimation;
                createLockAnimation(finishLockingAnimation, animMO);
                animMO.setId(AnimationId::FINISH_LOCKING);
            }

            {
                AnimationFrameSet cannotUnlockAnimation;
                cannotUnlockAnimation.loadPng(IDB_ANIM_CANNOT_UNLOCK, NumberOfFrames::CANNOT_UNLOCK);

                auto& animMC = res.body.menuClosed.cannotUnlockAnimation;
                animMC.createWithBackground(cannotUnlockAnimation.bitmap(), cannotUnlockAnimation.size(), bg);
                animMC.setPosition(model.lock.x, model.lock.y);
                animMC.setId(AnimationId::CANNOT_UNLOCK);

                auto& animMO = res.body.menuOpen.cannotUnlockAnimation;
                createLockAnimation(cannotUnlockAnimation, animMO);
                animMO.setId(AnimationId::CANNOT_UNLOCK);
            }

            {
                AnimationFrameSet unlockAnimation;
                unlockAnimation.loadPng(IDB_ANIM_UNLOCK, NumberOfFrames::UNLOCK);

                auto& animMC = res.body.menuClosed.unlockAnimation;
                animMC.createWithBackground(unlockAnimation.bitmap(), unlockAnimation.size(), bg);
                animMC.setPosition(model.lock.x, model.lock.y);
                animMC.setId(AnimationId::UNLOCK);

                auto& animMO = res.body.menuOpen.unlockAnimation;
                createLockAnimation(unlockAnimation, animMO);
                animMO.setId(AnimationId::UNLOCK);
            }
        }
    }

    void stopCurrentAnimation() {
        auto&& ah = animationHandler_;
        if (!ah.empty()) {
            auto& animation = ah.animation();

            if (animation.isFirstFrame()) {
                beforeAnimation_(animation, true);
            }

            ah.drawLastFrame(view().buffer_);
            ah.free();
            afterAnimation_(animation, true);

            view().needsUpdate_ = true;
        }
    }

    void runAnimation(AnimationFrameSet& animation, DWORD delayBeforeRun = 0) {
        stopCurrentAnimation();
        animationHandler_.runAnimation(animation, delayBeforeRun);
    }

    void setBeforeAnimationCallback(
            std::function<void(const AnimationFrameSet& animation, bool isAbort)>&& beforeAnimation) {
        beforeAnimation_ = std::move(beforeAnimation);
    }

    void setAfterAnimationCallback(
            std::function<void(const AnimationFrameSet& animation, bool isAbort)>&& afterAnimation) {
        afterAnimation_ = std::move(afterAnimation);
    }

    bool drawAnimation() {
        auto&& ah = animationHandler_;

        if (ah.ready()) {
            auto& animation = ah.animation();
            if (animation.isFirstFrame()) {
                beforeAnimation_(animation, false);
            }

            bool finished = ah.draw(view().buffer_);

            if (finished) {
                ah.free();
                afterAnimation_(animation, false);
                view().draw();
            }
            return true;
        }
        return false;
    }

    bool isAnimationActive() {
        return animationHandler_.active();
    }

    void createLockAnimation(AnimationFrameSet& animPlane, AnimationFrameSet& result) {
        auto& res = view().resources_;
        const auto& model = view().model_;

        auto&& scene = res.scene;
        auto frame3 = Rectangle::create(83, 14, 44, 70);
        int fw = Rectangle::width(frame3);
        int fh = Rectangle::height(frame3);

        unsigned w = animPlane.width();
        unsigned h = animPlane.height();
        unsigned frames = animPlane.size();

        Bitmap buf, tex;
        buf.create(fw, fh);
        tex.create(w, h);
        result.recreate(fw, fh, frames);

        FlyoutLayers layers = FlyoutLayers::all();
        layers.lockUpdateMaterial = false;

        auto& mat = scene.lock.material(FlyoutResources::LOCK_BUFFER_MAT);
        scene.lockGeo.setMaterialIndex(0, FlyoutResources::LOCK_BUFFER_MAT);

        animPlane.begin();
        for (unsigned i = 0; i < frames; i++) {
            animPlane.drawFrame(tex, 0, 0);

            mat.create(tex);
            view().renderBackground(1, layers, true);

            auto& rc = res.scene.viewFrame;
            buf.copyFrom(res.scene.renderBuffer, 0, 0, rc.left + frame3.left, rc.top + frame3.top,
                         fw, fh);
            result.addFrame(i, buf);

            animPlane.nextFrame();
        }

        result.setPosition(model.body.x + frame3.left, model.body.y + frame3.top);
    }

    void createMenuAnimation(AnimationFrameSet& result, const AnimationFrameSet& baseAnimation,
                             const FlyoutResources::MenuAnimationValues& values) {
        result.clone(baseAnimation);

        FlyoutLayers layers = FlyoutLayers::none();
        layers.lock = true;
        layers.progress = true;
        layers.keyboard = true;
        layers.mouse = true;

        const auto& res = view().resources_;
        const auto& viewFrame = res.scene.viewFrame;
        const int width = Rectangle::width(viewFrame);
        const int height = Rectangle::height(viewFrame);

        Bitmap frame;
        frame.create(width, height);
        FrameSet anim;
        anim.recreate(width, height, values.size());

        for (unsigned i = 0; i < values.size(); i++) {
            float v = values[i];

            view().renderBackground(v, layers, false);
            frame.copyFrom(res.scene.renderBuffer, 0, 0, viewFrame.left, viewFrame.top,
                           width, height);

            anim.addFrame(i, frame);
        }

        result.bitmap().blendFrom(anim.bitmap(), 0, 0);
        result.begin();
    }

    void createShowMenuAnimation() {
        auto& res = view().resources_;
        const auto& model = view().model_;
        auto&& anim = res.showMenuAnimation;
        createMenuAnimation(anim, res.openMenuAnimation, res.openMenuValues);
        anim.setId(AnimationId::SHOW_MENU);
        anim.setPosition(model.body.x, model.body.y);
        runAnimation(anim);
    }

    void createHideMenuAnimation() {
        auto& res = view().resources_;
        const auto& model = view().model_;
        auto&& anim = res.hideMenuAnimation;
        createMenuAnimation(anim, res.closeMenuAnimation, res.closeMenuValues);
        anim.setId(AnimationId::HIDE_MENU);
        anim.setPosition(model.body.x, model.body.y);
        runAnimation(anim);
    }

    void createShowFlyoutAnimation(AnimationAction actionBefore, AnimationAction actionAfter) {
        auto& res = view().resources_;
        const auto& model = view().model_;

        Bitmap buf;
        buf.createAs(view().buffer_);
        view().draw(buf, false, view().isMenuBarVisible() ? 1.0f : 0.0f);

        auto&& anim = res.showFlyoutAnimation;
        ShowWindowAnimation::create(anim, buf, model.workArea(), true);

        anim.setId(AnimationId::SHOW_MAIN_WINDOW);
        anim.setActionBefore(actionBefore);
        anim.setActionAfter(actionAfter);

        runAnimation(anim);
    }

    void createHideFlyoutAnimation(AnimationAction actionBefore, AnimationAction actionAfter) {
        auto& res = view().resources_;
        const auto& model = view().model_;

        Bitmap buf;
        buf.clone(view().buffer_);

        auto&& anim = res.hideFlyoutAnimation;
        ShowWindowAnimation::create(anim, buf, model.workArea(), false);

        anim.setId(AnimationId::HIDE_MAIN_WINDOW);
        anim.setActionBefore(actionBefore);
        anim.setActionAfter(actionAfter);

        runAnimation(anim);
    }

    void createShakeAnimation() {
        auto& res = view().resources_;
        const auto& model = view().model_;

        Bitmap buf;
        buf.createAs(view().buffer_);
        view().draw(buf, false, view().isMenuBarVisible() ? 1.0f : 0.0f);

        auto&& anim = res.shakeAnimation;
        ShakeAnimation::create(anim, buf, model.workArea());
        anim.setId(AnimationId::SHAKE);
        runAnimation(anim);

        Sounds::play(IDB_SND_SHAKE);
    }

    void createShowGuardAnimation() {
        Bitmap buf;
        buf.createAs(view().buffer_);
        view().draw(buf, false, view().isMenuBarVisible() ? 1.0f : 0.0f, false);
        auto& anim = view().guard_.createShowAnimation(buf);
        anim.setId(AnimationId::SHOW_GUARD);
        runAnimation(anim);
    }

    void createHideGuardAnimation() {
        Bitmap buf;
        buf.createAs(view().buffer_);

        view().draw(buf, false, view().isMenuBarVisible() ? 1.0f : 0.0f, false);
        auto& anim = view().guard_.createHideAnimation(buf);
        anim.setId(AnimationId::HIDE_GUARD);
        runAnimation(anim);
    }

    void createGuardWrongAnimation() {
        Bitmap buf;
        buf.createAs(view().buffer_);

        view().draw(buf, false, view().isMenuBarVisible() ? 1.0f : 0.0f, false);
        auto& anim = view().guard_.createWrongAnimation(buf);
        anim.setId(AnimationId::GUARD_WRONG);
        runAnimation(anim);
    }

    void runStartupAnimation() {
        auto&& anim = view().resources_.startupAnimation;
        anim.begin();
        runAnimation(anim);
    }

    void runStartLockingAnimation() {
        auto& res = view().resources_;
        const auto& model = view().model_;

        stopCurrentAnimation();
        view().resetGuard();

        if (model.lockState() != LockState::READY) {
            return;
        }

        Sounds::play(IDB_SND_START_LOCKING);
        if (view().isMenuBarVisible()) {
            auto&& anim = res.body.menuOpen.startLockingAnimation;
            anim.begin();
            runAnimation(anim);
        } else {
            auto&& anim = res.body.menuClosed.startLockingAnimation;
            anim.begin();
            runAnimation(anim);
        }
    }

    void runFinishLockingAnimation(bool lockNow) {
        auto& res = view().resources_;
        const auto& model = view().model_;

        stopCurrentAnimation();
        view().resetGuard();

        if (lockNow) {
            if (model.lockState() != LockState::READY) {
                return;
            }
        } else {
            if (model.lockState() != LockState::COUNTDOWN) {
                return;
            }
        }

        Sounds::play(IDB_SND_FINISH_LOCKING);
        DWORD delay = 50;
        if (view().isMenuBarVisible()) {
            auto&& anim = res.body.menuOpen.finishLockingAnimation;
            anim.begin();
            runAnimation(anim, delay);
        } else {
            auto&& anim = res.body.menuClosed.finishLockingAnimation;
            anim.begin();
            runAnimation(anim, delay);
        }
    }

    void runCannotUnlockAnimation() {
        auto& res = view().resources_;
        const auto& model = view().model_;

        stopCurrentAnimation();
        view().resetGuard();

        if (model.lockState() != LockState::LOCKED) {
            return;
        }

        Sounds::play(IDB_SND_WRONG);
        if (view().isMenuBarVisible()) {
            auto&& anim = res.body.menuOpen.cannotUnlockAnimation;
            anim.begin();
            runAnimation(anim);
        } else {
            auto&& anim = res.body.menuClosed.cannotUnlockAnimation;
            anim.begin();
            runAnimation(anim);
        }
    }

    void runUnlockAnimation() {
        auto& res = view().resources_;
        const auto& model = view().model_;

        stopCurrentAnimation();
        if (model.guard.visible()) {
            view().resetGuard();
        }

        if (model.lockState() != LockState::LOCKED) {
            return;
        }

        Sounds::play(IDB_SND_UNLOCK);
        if (view().isMenuBarVisible()) {
            auto&& anim = res.body.menuOpen.unlockAnimation;
            anim.begin();
            runAnimation(anim);
        } else {
            auto&& anim = res.body.menuClosed.unlockAnimation;
            anim.begin();
            runAnimation(anim);
        }
    }

private:
    AnimationHandler animationHandler_;
    std::function<void(const AnimationFrameSet& animation, bool isAbort)> beforeAnimation_;
    std::function<void(const AnimationFrameSet& animation, bool isAbort)> afterAnimation_;
};

} // namespace litelockr

#endif // FLYOUT_ANIMATION_H
