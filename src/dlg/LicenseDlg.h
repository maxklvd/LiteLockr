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

#ifndef LICENSE_DLG_H
#define LICENSE_DLG_H

#include "gui/Dialog.h"

namespace litelockr {

class LicenseDlg: public Dialog {
public:
    LicenseDlg();

    void show();
    void destroy();

protected:
    BOOL onInitDialog() override;
    void onShowDialog() override {}
    void localizeDialog();
};

} // namespace litelockr

#endif // LICENSE_DLG_H
