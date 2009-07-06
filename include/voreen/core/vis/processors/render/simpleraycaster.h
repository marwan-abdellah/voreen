/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
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

#ifndef VRN_SIMPLERAYCASTER_H
#define VRN_SIMPLERAYCASTER_H

#include "voreen/core/vis/processors/render/volumeraycaster.h"

namespace voreen {

/**
 * Performs a simple single pass raycasting with only some capabilites.
 */
class SimpleRaycaster : public VolumeRaycaster {
public:

    /**
     * Constructor.
     *
     */
    SimpleRaycaster();

    virtual ~SimpleRaycaster();

    virtual const Identifier getClassName() const {return "Raycaster.SimpleRaycaster";}
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const {return new SimpleRaycaster();}

    virtual int initializeGL();

    /**
     * Load the needed shader.
     *
     */
    virtual void loadShader();

    /**
     * Performs the raycasting.
     *
     * Initialize two texture units with the entry and exit params and renders
     * a screen aligned quad. The render destination is determined by the
     * invoking class.
     */
    virtual void process(LocalPortMapping*  portMapping);

protected:
    virtual std::string generateHeader();

    virtual void compile();

private:
    TransFuncProp transferFunc_;  ///< the property that controls the transfer-function
};


} // namespace

#endif // VRN_SIMPLERAYCASTER_H
