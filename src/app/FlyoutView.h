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

#ifndef FLYOUT_VIEW_H
#define FLYOUT_VIEW_H

#include <functional>

#include "app/FlyoutButtonOpacity.h"
#include "app/FlyoutLayers.h"
#include "app/FlyoutModel.h"
#include "app/FlyoutProgressBar.h"
#include "app/FlyoutResources.h"
#include "app/animation/FlyoutAnimation.h"
#include "app/guard/UnlockGuardView.h"

namespace litelockr {

class FlyoutView: public FlyoutAnimation<FlyoutView> {
public:
    FlyoutView(Bitmap& buffer, FlyoutModel& model)
            : buffer_(buffer),
              model_(model),
              guard_(buffer, model.guard) {}

    ~FlyoutView() = default;

    [[nodiscard]] bool hasInitialized() const { return hasInitialized_; }

    void initialize(HWND hWnd);

    void setScenePlaneLockVisible(bool visible);
    void drawProgress(Bitmap& buffer, float progress) const;
    void checkAction(AnimationAction actionBefore);

    bool needsUpdate();

    void draw();
    void draw(Bitmap& buffer, bool isHidden, float menuBar = 0, bool showGuard = true);

    void updateOptButtons(Bitmap& buffer);
    unsigned getLockMaterialIndex();

    void renderBackground(float value, FlyoutLayers layers, bool useCache);
    void drawBackground(Bitmap& buffer) const;
    static void setForeground(HWND hWnd);

    void setGuardActiveState(UnlockGuardActiveState state);
    [[nodiscard]] bool isGuardVisible() const { return model_.visible() && model_.guard.visible(); }
    void resetGuard();
    [[nodiscard]] static bool isProtected();
    [[nodiscard]] bool isMenuBarVisible() const { return model_.menuBar.visible; }

    void updateButtonMaterial();
    bool updateButtonOpacity();

    FlyoutProgressBar& getProgressBar() { return progressBar_; }

    void runLocking(bool start);

    const FlyoutResources& resources() const { return resources_; }

private:
    void initializeImages();
    void initializeScene();
    void checkImageSizes();
    void drawMenuBarMaterials();

    bool hasInitialized_ = false;
    bool needsUpdate_ = false;

    Bitmap& buffer_;
    FlyoutModel& model_;
    UnlockGuardView guard_;

    FlyoutResources resources_;
    FlyoutButtonOpacity buttonOpacity_;
    FlyoutProgressBar progressBar_;

    enum MenuButtonOpacity {
        BUTTON_NORMAL = 180u,
        BUTTON_HOVER = 200u,
        BUTTON_PRESSED = 140u,
    };

    friend class FlyoutAnimation;
};

} // namespace litelockr

#endif // FLYOUT_VIEW_H
