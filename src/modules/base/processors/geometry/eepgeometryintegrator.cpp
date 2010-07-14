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

#include "voreen/modules/base/processors/geometry/eepgeometryintegrator.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

EEPGeometryIntegrator::EEPGeometryIntegrator()
    : ImageProcessor("eep_geometry")
    , inport0_(Port::INPORT, "image.entry")
    , inport1_(Port::INPORT, "image.exit")
    , geometryPort_(Port::INPORT, "image.geometry")
    , entryPort_(Port::OUTPORT, "image.postentry", true, Processor::INVALID_PROGRAM)
    , exitPort_(Port::OUTPORT, "image.postexit", true, Processor::INVALID_PROGRAM)
    , tmpPort_(Port::OUTPORT, "image.tmp", false)
    , useFloatRenderTargets_("useFloatRenderTargets", "Use float rendertargets", false)
    , camera_("camera", "Camera", new tgt::Camera(tgt::vec3(0.f, 0.f, 3.5f), tgt::vec3(0.f, 0.f, 0.f), tgt::vec3(0.f, 1.f, 0.f)))
{
    addPort(inport0_);
    addPort(inport1_);
    addPort(geometryPort_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPrivateRenderPort(&tmpPort_);

    addProperty(useFloatRenderTargets_);
    addProperty(camera_);
}

EEPGeometryIntegrator::~EEPGeometryIntegrator() {
}

std::string EEPGeometryIntegrator::getProcessorInfo() const {
    return "Combines entry-exit-parameters for a raycaster with geometry.";
}

Processor* EEPGeometryIntegrator::create() const {
    return new EEPGeometryIntegrator();
}

void EEPGeometryIntegrator::initialize() throw (VoreenException) {
    ImageProcessor::initialize();//FIXME: twice?

    //loadShader();
}

void EEPGeometryIntegrator::loadShader() {
    //program_->rebuild();
}

void EEPGeometryIntegrator::compileShader() {
    //program_->setHeader(generateHeader());
    //program_->rebuild();
}

void EEPGeometryIntegrator::process() {
    //TODO: move somewhere else (stefan)
    if(useFloatRenderTargets_.get()) {
        if(entryPort_.getData()->getColorTexture()->getDataType() != GL_FLOAT) {
            tgt::ivec2 s = entryPort_.getSize();

            entryPort_.setData(new RenderTarget());
            entryPort_.getData()->initialize(GL_RGBA16F_ARB);
            entryPort_.getData()->resize(s);


            exitPort_.setData(new RenderTarget());
            exitPort_.getData()->initialize(GL_RGBA16F_ARB);
            exitPort_.getData()->resize(s);

            tmpPort_.setData(new RenderTarget());
            tmpPort_.getData()->initialize(GL_RGBA16F_ARB);
            tmpPort_.getData()->resize(s);
        }
    }
    else {
        if(entryPort_.getData()->getColorTexture()->getDataType() == GL_FLOAT) {
            tgt::ivec2 s = entryPort_.getSize();

            entryPort_.setData(new RenderTarget());
            entryPort_.getData()->initialize(GL_RGBA16);
            entryPort_.getData()->resize(s);


            exitPort_.setData(new RenderTarget());
            exitPort_.getData()->initialize(GL_RGBA16);
            exitPort_.getData()->resize(s);

            tmpPort_.setData(new RenderTarget());
            tmpPort_.getData()->initialize(GL_RGBA16);
            tmpPort_.getData()->resize(s);
        }
    }

    // compile program if needed
   // if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
     //   compileShader();
    //LGL_ERROR;

    entryPort_.activateTarget();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TextureUnit shadeUnit0, shadeUnitDepth0, shadeUnit1, shadeUnitDepth1, shadeUnit2, shadeUnitDepth2;

    inport0_.bindTextures(shadeUnit0.getEnum(), shadeUnitDepth0.getEnum());
    inport1_.bindTextures(shadeUnit1.getEnum(), shadeUnitDepth1.getEnum());
    geometryPort_.bindTextures(shadeUnit2.getEnum(), shadeUnitDepth2.getEnum());

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_, camera_.get());
    program_->setIgnoreUniformLocationError(true);
    program_->setUniform("entryParams_", shadeUnit0.getUnitNumber());
    program_->setUniform("entryParamsDepth_", shadeUnitDepth0.getUnitNumber());
    program_->setUniform("exitParams_", shadeUnit1.getUnitNumber());
    program_->setUniform("exitParamsDepth_", shadeUnitDepth1.getUnitNumber());
    program_->setUniform("geometryTex_", shadeUnit2.getUnitNumber());
    program_->setUniform("geometryTexDepth_", shadeUnitDepth2.getUnitNumber());

    inport0_.setTextureParameters(program_, "entryInfo_");
    inport1_.setTextureParameters(program_, "exitInfo_");
    geometryPort_.setTextureParameters(program_, "geomInfo_");
    program_->setIgnoreUniformLocationError(false);

    program_->setUniform("entry_", true);

    glDepthFunc(GL_ALWAYS);
    renderQuad();
    glDepthFunc(GL_LESS);

    exitPort_.activateTarget();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program_->setUniform("entry_", false);
    program_->setUniform("near_", camera_.get()->getNearDist());
    program_->setUniform("far_", camera_.get()->getFarDist());
    program_->setUniform("useFloatTarget_", useFloatRenderTargets_.get());

    glDepthFunc(GL_ALWAYS);
    renderQuad();
    glDepthFunc(GL_LESS);

    program_->deactivate();
    glActiveTexture(GL_TEXTURE0);
    LGL_ERROR;
}

} // voreen namespace
