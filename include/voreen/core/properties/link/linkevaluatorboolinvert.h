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

#ifndef VRN_LINKEVALUATORBOOLINVERT_H
#define VRN_LINKEVALUATORBOOLINVERT_H

#include "voreen/core/properties/link/linkevaluatorbase.h"

namespace voreen {

/**
 * Propagates an inverted boolean value.
 */
class LinkEvaluatorBoolInvert : public LinkEvaluatorBase {
public:

    virtual BoxObject eval(const BoxObject& sourceOld, const BoxObject& sourceNew, const BoxObject& targetOld, Property* src, Property* dest);

    virtual std::string name() const;
};

} // namespace

#endif // VRN_LINKEVALUATORBOOLINVERT_H