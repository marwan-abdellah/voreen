/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Created between 2005 and 2012 by The Voreen Team                   *
 * as listed in CREDITS.TXT <http://www.voreen.org>                   *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#ifndef VRN_CAMERAPROPERTYWIDGET_H
#define VRN_CAMERAPROPERTYWIDGET_H

#include "voreen/qt/widgets/property/qpropertywidgetwitheditorwindow.h"

class QPushButton;

namespace tgt {
    class Camera;
}
namespace voreen {

class CameraProperty;
class VoreenToolWindow;
class CameraWidget;

class CameraPropertyWidget : public QPropertyWidgetWithEditorWindow {
    Q_OBJECT
public:
    CameraPropertyWidget(CameraProperty* prop, QWidget* parent = 0);

    void updateFromProperty();

public slots:
    void setProperty(tgt::Camera* value);

private slots:
    void toggleWidgetVisibility();

protected:
    virtual QWidget* createEditorWindowWidget();
    virtual void customizeEditorWindow();
    virtual Property* getProperty();

    QPushButton* editBt_;

    CameraProperty* property_;
    CameraWidget* cameraWidget_;
};

} // namespace

#endif // VRN_CAMERAPROPERTYWIDGET_H
