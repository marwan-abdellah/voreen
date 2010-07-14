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

#ifndef VRN_HALFANGLESLICER_H
#define VRN_HALFANGLESLICER_H

#include "voreen/modules/base/processors/render/volumeslicer.h"
#include "voreen/core/interaction/camerainteractionhandler.h"

namespace voreen {

class HalfAngleSlicer : public VolumeSlicer {
public:
    /**
     * Constructor.
     */
    HalfAngleSlicer();

    virtual std::string getCategory() const { return "Slice Rendering"; }
    virtual std::string getClassName() const { return "HalfAngleSlicer"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_TESTING; }
    virtual std::string getProcessorInfo() const;
    virtual Processor* create() const { return new HalfAngleSlicer(); }

    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

    /**
     * Performs the slicing.
     */
    virtual void process();

    virtual bool isReady() const;

protected:

    /**
     * Load the needed shaders.
     */
    virtual void loadShader();

    virtual std::string generateHeader();
    virtual void compile();

private:

    tgt::Shader* slicingPrg_;

    VolumePort volumeInport_;
    RenderPort outport_;
    RenderPort lightport_;

    // interaction handlers
    CameraInteractionHandler cameraHandler_;

    CameraProperty lightCamera_;
    LightProperty halfLight_;
};


} // namespace voreen

#endif // VRN_HALFANGLESLICER_H
