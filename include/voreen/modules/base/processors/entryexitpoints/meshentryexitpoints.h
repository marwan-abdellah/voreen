/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 The Voreen Team. <http://www.voreen.org>   *
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

#ifndef VRN_MESHENTRYEXITPOINTS_H
#define VRN_MESHENTRYEXITPOINTS_H

#include "voreen/core/processors/renderprocessor.h"
#include "voreen/core/datastructures/geometry/meshlistgeometry.h"

#include "tgt/shadermanager.h"

namespace voreen {

class CameraInteractionHandler;

/**
 * Calculates the entry and exit points for GPU raycasting and stores them in textures.
 *
 * @see CubeMeshProxyGeometry, SingleVolumeRaycaster
 */
class MeshEntryExitPoints : public RenderProcessor {
public:
    MeshEntryExitPoints();
    virtual ~MeshEntryExitPoints();

    virtual std::string getClassName() const    { return "MeshEntryExitPoints"; }
    virtual std::string getCategory() const     { return "EntryExitPoints"; }
    virtual CodeState getCodeState() const      { return CODE_STATE_STABLE; }

    virtual std::string getProcessorInfo() const;
    virtual Processor* create() const;
    virtual bool isReady() const;

protected:
    virtual void process();

    virtual void initialize() throw (VoreenException);

    virtual void beforeProcess();

    /**
     *  Jitters entry points in ray direction.
     *  Entry and Exit Params have to be generated before
     *  calling this method.
     */
    void jitterEntryPoints();

    /// (Re-)generates jitter texture
    void generateJitterTexture();

    void onJitterEntryPointsChanged();
    void onFilterJitterTextureChanged();

    MeshListGeometry geometry_;

    tgt::Shader* shaderProgram_;
    tgt::Shader* shaderProgramInsideVolume_;
    tgt::Shader* shaderProgramJitter_;
    tgt::Shader* shaderProgramClipping_;

    // processor properties
    BoolProperty supportCameraInsideVolume_;
    BoolProperty jitterEntryPoints_;
    BoolProperty filterJitterTexture_;
    BoolProperty useFloatRenderTargets_;
    FloatProperty jitterStepLength_;
    tgt::Texture* jitterTexture_;
    CameraProperty camera_;  ///< camera used for rendering the proxy geometry

    // interaction handlers
    CameraInteractionHandler* cameraHandler_;

    // ports
    RenderPort entryPort_;
    RenderPort exitPort_;
    GeometryPort inport_;
    RenderPort tmpPort_;

    /// Category used for logging.
    static const std::string loggerCat_;
};

} // namespace voreen

#endif //VRN_MESHENTRYEXITPOINTS_H
