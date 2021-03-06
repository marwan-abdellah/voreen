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

#ifndef VRN_GRADIENT_H
#define VRN_GRADIENT_H

#include "voreen/core/voreencoredefine.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumehandle.h"

#include <string>
#include <iostream>
#include <fstream>

using tgt::ivec3;
using tgt::svec3;
using tgt::vec3;

namespace voreen {

//Max gradient = max change over min distance:
template<typename T>
float getMaxGradientLength(tgt::vec3 spacing) {
    return (VolumeElement<T>::rangeMaxElement() - VolumeElement<T>::rangeMinElement()) / min(spacing);
}

//Store gradient in volume.
template<typename U>
void storeGradient(tgt::vec3 gradient, const tgt::ivec3& pos, VolumeAtomic<tgt::Vector3<U> >* result) {
    if(VolumeElement<tgt::Vector3<U> >::isInteger()) {
        //map to [0,1]:
        gradient += 1.0f;
        gradient /= 2.0f;

        //...and to [minElement,maxElement]:
        gradient *= VolumeElement<U>::rangeMaxElement() - VolumeElement<U>::rangeMinElement();
        gradient += VolumeElement<U>::rangeMinElement();

        result->voxel(pos) = tgt::Vector3<U>(static_cast<U>(gradient.x), static_cast<U>(gradient.y), static_cast<U>(gradient.z));
    }
    else {
        //floating point output, no remapping:
        result->voxel(pos) = tgt::Vector3<U>(static_cast<U>(gradient.x), static_cast<U>(gradient.y), static_cast<U>(gradient.z));
    }
}

template<class T>
vec3 calcGradientCentralDifferences(const VolumeAtomic<T>* input, const tgt::vec3& spacing, const tgt::svec3& pos) {
    T v0, v1, v2, v3, v4, v5;
    vec3 gradient;

    if (pos.x != input->getDimensions().x-1)
        v0 = input->voxel(pos + tgt::svec3(1, 0, 0));
    else
        v0 = 0;
    if (pos.y != input->getDimensions().y-1)
        v1 = input->voxel(pos + tgt::svec3(0, 1, 0));
    else
        v1 = 0;
    if (pos.z != input->getDimensions().z-1)
        v2 = input->voxel(pos + tgt::svec3(0, 0, 1));
    else
        v2 = 0;

    if (pos.x != 0)
        v3 = input->voxel(pos + tgt::svec3(-1, 0, 0));
    else
        v3 = 0;
    if (pos.y != 0)
        v4 = input->voxel(pos + tgt::svec3(0, -1, 0));
    else
        v4 = 0;
    if (pos.z != 0)
        v5 = input->voxel(pos + tgt::svec3(0, 0, -1));
    else
        v5 = 0;

    gradient = tgt::vec3(static_cast<float>(v3 - v0), static_cast<float>(v4 - v1), static_cast<float>(v5 - v2));
    gradient /= (spacing * 2.0f);

    return gradient;
}

template<class U, class T>
VolumeHandle* calcGradientsCentralDifferences(const VolumeHandleBase* handle) {
    const VolumeAtomic<T>* input = dynamic_cast<const VolumeAtomic<T>*>(handle->getRepresentation<Volume>());
    VolumeAtomic<tgt::Vector3<U> >* result = new VolumeAtomic<tgt::Vector3<U> >(input->getDimensions());

    tgt::vec3 gradient;
    tgt::svec3 pos;
    tgt::svec3 dim = input->getDimensions();
    tgt::vec3 spacing = handle->getSpacing();

    //We normalize gradients for integer datasets:
    bool normalizeGradient = VolumeElement<T>::isInteger();
    float maxGradientLength = 1.0f;
    if(normalizeGradient) {
        maxGradientLength = getMaxGradientLength<T>(handle->getSpacing());
    }

    for (pos.z = 0; pos.z < dim.z; pos.z++) {
        for (pos.y = 0; pos.y < dim.y; pos.y++) {
            for (pos.x = 0; pos.x < dim.x; pos.x++) {
                gradient = calcGradientCentralDifferences(input, spacing, pos);

                if(normalizeGradient)
                    gradient /= maxGradientLength;

                storeGradient(gradient, pos, result);
            }
        }
    }
    return new VolumeHandle(result, handle);
}


/**
 * Calculates gradients by central differences.
 *
 * Returns a VolumeHandle with a VolumeAtomic<Vector3<U>> Volume.
 */
template<class U>
VolumeHandle* calcGradientsCentralDifferences(const VolumeHandleBase* volh) {
    const Volume* vol = volh->getRepresentation<Volume>();

    if (dynamic_cast<const VolumeUInt8*>(vol))
        return calcGradientsCentralDifferences<U, uint8_t>(volh);
    else if (dynamic_cast<const VolumeUInt16*>(vol))
        return calcGradientsCentralDifferences<U, uint16_t>(volh);
    if (dynamic_cast<const VolumeFloat*>(vol))
        return calcGradientsCentralDifferences<U, float>(volh);
    else {
        LERRORC("calcGradientsCentralDifferences", "Unsupported input");
    }

    return 0;
}

/**
 * Calculates gradients using linear regression according to Neumann et al.
 *
 \verbatim
    L. Neumann, B. Csbfalvi, A. Koenig, and M. E. Groeller.
    Gradient estimation in volume data using 4D linear regression.
    In Proceedings of Eurographics 2000, pages 351-358, 2000.
 \endverbatim
 *
 * The neighboring voxels are weighted by their reciprocal Euclidean distance.
 *
 * Returns a VolumeHandle with a VolumeAtomic<Vector3<U>> Volume.
 */

template<class T>
vec3 calcGradientLinearRegression(const VolumeAtomic<T>* input, const tgt::vec3& spacing, const tgt::ivec3& pos) {
    vec3 gradient;
    
    // Euclidean weights for voxels with Manhattan distances of 1/2/3
    float w_1 = 1.f;
    float w_2 = 0.5f;
    float w_3 = 1.f/3.f;

    float w_A = 1.f / (8.f + 2.f/3.f);
    float w_B = w_A;
    float w_C = w_A;

    if (pos.x >= 1 && pos.x < tgt::ivec3(input->getDimensions()).x-1 &&
        pos.y >= 1 && pos.y < tgt::ivec3(input->getDimensions()).y-1 &&
        pos.z >= 1 && pos.z < tgt::ivec3(input->getDimensions()).z-1)
    {
        //left plane
        T v000 = input->voxel(pos + ivec3(-1,-1,-1));
        T v001 = input->voxel(pos + ivec3(-1, -1, 0));
        T v002 = input->voxel(pos + ivec3(-1, -1, 1));
        T v010 = input->voxel(pos + ivec3(-1, 0, -1));
        T v011 = input->voxel(pos + ivec3(-1, 0, 0));
        T v012 = input->voxel(pos + ivec3(-1, 0, 1));
        T v020 = input->voxel(pos + ivec3(-1, 1, -1));
        T v021 = input->voxel(pos + ivec3(-1, 1, 0));
        T v022 = input->voxel(pos + ivec3(-1, 1, 1));

        //mid plane
        T v100 = input->voxel(pos + ivec3(0, -1, -1));
        T v101 = input->voxel(pos + ivec3(0, -1, 0));
        T v102 = input->voxel(pos + ivec3(0, -1, 1));
        T v110 = input->voxel(pos + ivec3(0, 0, -1));
        //T v111 = input->voxel(pos + ivec3(0, 0, 0));
        T v112 = input->voxel(pos + ivec3(0, 0, 1));
        T v120 = input->voxel(pos + ivec3(0, 1, -1));
        T v121 = input->voxel(pos + ivec3(0, 1, 0));
        T v122 = input->voxel(pos + ivec3(0, 1, 1));

        //right plane
        T v200 = input->voxel(pos + ivec3(1, -1, -1));
        T v201 = input->voxel(pos + ivec3(1, -1, 0));
        T v202 = input->voxel(pos + ivec3(1, -1, 1));
        T v210 = input->voxel(pos + ivec3(1, 0, -1));
        T v211 = input->voxel(pos + ivec3(1, 0, 0));
        T v212 = input->voxel(pos + ivec3(1, 0, 1));
        T v220 = input->voxel(pos + ivec3(1, 1, -1));
        T v221 = input->voxel(pos + ivec3(1, 1, 0));
        T v222 = input->voxel(pos + ivec3(1, 1, 1));

        gradient.x = static_cast<float>( w_1 * ( v211 - v011 )               +
                w_2 * ( v201 + v210 + v212 + v221
                    -v001 - v010 - v012 - v021 ) +
                w_3 * ( v200 + v202 + v220 + v222
                    -v000 - v002 - v020 - v022 )   );

        gradient.y = static_cast<float>( w_1 * ( v121 - v101 )               +
                w_2 * ( v021 + v120 + v122 + v221
                    -v001 - v100 - v102 - v201 ) +
                w_3 * ( v020 + v022 + v220 + v222
                    -v000 - v002 - v200 - v202 )   );

        gradient.z = static_cast<float>( w_1 * ( v112 - v110 )               +
                w_2 * ( v012 + v102 + v122 + v212
                    -v010 - v100 - v120 - v210 ) +
                w_3 * ( v002 + v022 + v202 + v222
                    -v000 - v020 - v200 - v220 )   );

        gradient.x *= w_A;
        gradient.y *= w_B;
        gradient.z *= w_C;

        gradient /= spacing;
        gradient *= -1.f;
    }
    else {
        gradient = vec3(0.f);
    }

    return gradient;
}
template<class U, class T>
VolumeHandle* calcGradientsLinearRegression(const VolumeHandleBase* handle) {
    const VolumeAtomic<T>* input = dynamic_cast<const VolumeAtomic<T>*>(handle->getRepresentation<Volume>());
    VolumeAtomic<tgt::Vector3<U> >* result = new VolumeAtomic<tgt::Vector3<U> >(input->getDimensions());

    //We normalize gradients for integer datasets:
    bool normalizeGradient = VolumeElement<T>::isInteger();
    float maxGradientLength = 1.0f;
    if(normalizeGradient) {
        maxGradientLength = getMaxGradientLength<T>(handle->getSpacing());
    }

    vec3 gradient;
    ivec3 pos;
    ivec3 dim = input->getDimensions();
    tgt::vec3 spacing = handle->getSpacing();

    for (pos.z = 0; pos.z < dim.z; pos.z++) {
        for (pos.y = 0; pos.y < dim.y; pos.y++) {
            for (pos.x = 0; pos.x < dim.x; pos.x++) {
                gradient = calcGradientLinearRegression(input, spacing, pos);

                if(normalizeGradient)
                    gradient /= maxGradientLength;

                storeGradient(gradient, pos, result);
            }
        }
    }

    return new VolumeHandle(result, handle);
}

template<class U>
VolumeHandle* calcGradientsLinearRegression(const VolumeHandleBase* handle) {
    const Volume* vol = handle->getRepresentation<Volume>();
    if (vol->getBitsStored() == 8) {
        return calcGradientsLinearRegression<U, uint8_t>(handle);
    }
    else if (vol->getBitsStored() == 12) {
        return calcGradientsLinearRegression<U, uint16_t>(handle);
    }
    else if (vol->getBitsStored() == 16) {
        return calcGradientsLinearRegression<U, uint16_t>(handle);
    }
    LERRORC("calcGradientsLinearRegression", "calcGradientsLinearRegression needs a 8-, 12- or 16-bit dataset as input");
    return 0;
}

template<class T>
vec3 calcGradientSobel(const VolumeAtomic<T>* input, const tgt::vec3& spacing, const tgt::ivec3& pos) {
    vec3 gradient = vec3(0.f);

    if (pos.x >= 1 && pos.x < tgt::ivec3(input->getDimensions()).x-1 &&
        pos.y >= 1 && pos.y < tgt::ivec3(input->getDimensions()).y-1 &&
        pos.z >= 1 && pos.z < tgt::ivec3(input->getDimensions()).z-1)
    {
        //left plane
        T v000 = input->voxel(pos + ivec3(-1, -1, -1));
        T v001 = input->voxel(pos + ivec3(-1, -1, 0));
        T v002 = input->voxel(pos + ivec3(-1, -1, 1));
        T v010 = input->voxel(pos + ivec3(-1, 0, -1));
        T v011 = input->voxel(pos + ivec3(-1, 0, 0));
        T v012 = input->voxel(pos + ivec3(-1, 0, 1));
        T v020 = input->voxel(pos + ivec3(-1, 1, -1));
        T v021 = input->voxel(pos + ivec3(-1, 1, 0));
        T v022 = input->voxel(pos + ivec3(-1, 1, 1));
        //mid plane
        T v100 = input->voxel(pos + ivec3(0, -1, -1));
        T v101 = input->voxel(pos + ivec3(0, -1, 0));
        T v102 = input->voxel(pos + ivec3(0, -1, 1));
        T v110 = input->voxel(pos + ivec3(0, 0, -1));
        //T v111 = input->voxel(pos + ivec3(0, 0, 0)); //not needed for calculation
        T v112 = input->voxel(pos + ivec3(0, 0, 1));
        T v120 = input->voxel(pos + ivec3(0, -1, -1));
        T v121 = input->voxel(pos + ivec3(0, -1, 0));
        T v122 = input->voxel(pos + ivec3(0, -1, 1));
        //right plane
        T v200 = input->voxel(pos + ivec3(1, -1, -1));
        T v201 = input->voxel(pos + ivec3(1, -1, 0));
        T v202 = input->voxel(pos + ivec3(1, -1, 1));
        T v210 = input->voxel(pos + ivec3(1, 0, -1));
        T v211 = input->voxel(pos + ivec3(1, 0, 0));
        T v212 = input->voxel(pos + ivec3(1, 0, 1));
        T v220 = input->voxel(pos + ivec3(1, 1, -1));
        T v221 = input->voxel(pos + ivec3(1, 1, 0));
        T v222 = input->voxel(pos + ivec3(1, 1, 1));

        //filter x-direction
        gradient.x += -1 * v000;
        gradient.x += -3 * v010;
        gradient.x += -1 * v020;
        gradient.x += 1 * v200;
        gradient.x += 3 * v210;
        gradient.x += 1 * v220;
        gradient.x += -3 * v001;
        gradient.x += -6 * v011;
        gradient.x += -3 * v021;
        gradient.x += +3 * v201;
        gradient.x += +6 * v211;
        gradient.x += +3 * v221;
        gradient.x += -1 * v002;
        gradient.x += -3 * v012;
        gradient.x += -1 * v022;
        gradient.x += +1 * v202;
        gradient.x += +3 * v212;
        gradient.x += +1 * v222;

        //filter y-direction
        gradient.y += -1 * v000;
        gradient.y += -3 * v100;
        gradient.y += -1 * v200;
        gradient.y += +1 * v020;
        gradient.y += +3 * v120;
        gradient.y += +1 * v220;
        gradient.y += -3 * v001;
        gradient.y += -6 * v101;
        gradient.y += -3 * v201;
        gradient.y += +3 * v021;
        gradient.y += +6 * v121;
        gradient.y += +3 * v221;
        gradient.y += -1 * v002;
        gradient.y += -3 * v102;
        gradient.y += -1 * v202;
        gradient.y += +1 * v022;
        gradient.y += +3 * v122;
        gradient.y += +1 * v222;

        //filter z-direction
        gradient.z += -1 * v000;
        gradient.z += -3 * v100;
        gradient.z += -1 * v200;
        gradient.z += +1 * v002;
        gradient.z += +3 * v102;
        gradient.z += +1 * v202;
        gradient.z += -3 * v010;
        gradient.z += -6 * v110;
        gradient.z += -3 * v210;
        gradient.z += +3 * v012;
        gradient.z += +6 * v112;
        gradient.z += +3 * v212;
        gradient.z += -1 * v020;
        gradient.z += -3 * v120;
        gradient.z += -1 * v220;
        gradient.z += +1 * v022;
        gradient.z += +3 * v122;
        gradient.z += +1 * v222;

        gradient /= 22.f;   // sum of all positive weights
        gradient /= 2.f;    // this mask has a step length of 2 voxels
        gradient /= spacing;
        gradient *= -1.f;
    }

    return gradient;
}

/**
 * Calculates gradients with neighborhood of 26, using the Sobel filter.
 * Returns a VolumeHandle with a VolumeAtomic<Vector3<U>> Volume.
 */
template<class U, class T>
VolumeHandle* calcGradientsSobel(const VolumeHandleBase* handle) {
    const VolumeAtomic<T>* input = dynamic_cast<const VolumeAtomic<T>*>(handle->getRepresentation<Volume>());
    VolumeAtomic<tgt::Vector3<U> >* result = new VolumeAtomic<tgt::Vector3<U> >(input->getDimensions());

    using tgt::vec3;
    using tgt::ivec3;

    //We normalize gradients for integer datasets:
    bool normalizeGradient = VolumeElement<T>::isInteger();
    float maxGradientLength = 1.0f;
    if(normalizeGradient) {
        maxGradientLength = getMaxGradientLength<T>(handle->getSpacing());
    }

    vec3 gradient;
    tgt::vec3 spacing = handle->getSpacing();
    ivec3 pos;
    ivec3 dim = input->getDimensions();

    for (pos.z = 0; pos.z < dim.z; pos.z++) {
        for (pos.y = 0; pos.y < dim.y; pos.y++) {
            for (pos.x = 0; pos.x < dim.x; pos.x++) {
                gradient = calcGradientSobel(input, spacing, pos);

                if(normalizeGradient)
                    gradient /= maxGradientLength;

                storeGradient(gradient, pos, result);
            }
        }
    }
    return new VolumeHandle(result, handle);
}

/**
 * Calculates gradients with Sobel operator.
 * Returns a VolumeHandle with a VolumeAtomic<Vector3<U>> Volume.
 */
template<class U>
VolumeHandle* calcGradientsSobel(const VolumeHandleBase* volh) {
    const Volume* vol = volh->getRepresentation<Volume>();
    if (vol->getBitsStored() == 8) {
        return calcGradientsSobel<U, uint8_t>(volh);
    }
    else if (vol->getBitsStored() == 16) {
        return calcGradientsSobel<U, uint16_t>(volh);
    }
    LERRORC("calcGradientsSobel", "calcGradientsSobel needs a 8-, 12- or 16-bit dataset as input");
    return 0;
}


/**
 * Calculates gradient magnitudes from a gradient volume.
 *
 * Use uint8_t or uint16_t as U template argument in order to generate 8 or 16 bit datasets.
 */
template<class U, class T>
VolumeAtomic<U>* calcGradientMagnitudesGeneric(const VolumeHandleBase* handle) {
    const VolumeAtomic<T>* input = dynamic_cast<const VolumeAtomic<T>*>(handle->getRepresentation<Volume>());

    VolumeAtomic<U>* result = new VolumeAtomic<U>(input->getDimensions());

    //float maxValueT;
    //if ( typeid(*input) == typeid(Volume3xUInt8)  ||
         //typeid(*input) == typeid(Volume3xUInt16) ||
         //typeid(*input) == typeid(Volume4xUInt8)  ||
         //typeid(*input) == typeid(Volume4xUInt16)    ) {
         //int bitsT = input->getBitsStored() / input->getNumChannels();
        //maxValueT = static_cast<float>( (1 << bitsT) - 1);
    //}
    //else if ( typeid(*input) == typeid(Volume3xFloat) || typeid(*input) == typeid(Volume3xDouble) ||
        //typeid(*input) == typeid(Volume4xFloat) || typeid(*input) == typeid(Volume4xDouble) ) {
        //maxValueT = 1.f;
    //}
    //else {
        //LERRORC("calcGradientMagnitudes", "Unknown or unsupported input volume type");
        //tgtAssert(false, "Unknown or unsupported input volume type");
        //return result;
    //}

    float maxValueU;
    if ( typeid(*result) == typeid(VolumeUInt8)  ||
         typeid(*result) == typeid(VolumeUInt16) ||
         typeid(*result) == typeid(VolumeUInt32) )
        maxValueU = static_cast<float>( (1 << result->getBitsStored()) - 1);
    else if ( typeid(*result) == typeid(VolumeFloat) || typeid(*result) == typeid(VolumeDouble))
        maxValueU = 1.f;
    else {
        LERRORC("calcGradientMagnitudes", "Unknown or unsupported output volume type");
        tgtAssert(false, "Unknown or unsupported output volume type");
        return result;
    }


    ivec3 pos;
    ivec3 dim = input->getDimensions();
    for (pos.z = 0; pos.z < dim.z; pos.z++) {
        for (pos.y = 0; pos.y < dim.y; pos.y++) {
            for (pos.x = 0; pos.x < dim.x; pos.x++) {

                vec3 gradient;
                gradient.x = input->getVoxelFloat(pos, 0);
                gradient.y = input->getVoxelFloat(pos, 1);
                gradient.z = input->getVoxelFloat(pos, 2);

                // input value range is [0:maxValue] with (maxValue/2.f) corresponding to zero
                gradient = (gradient*2.f)-1.f;

                float gradientMagnitude = tgt::length(gradient);

                //result->voxel(pos) = static_cast<U>( ( (derivative / maxValueT) / 2.f + 0.5f ) * maxValueU );
                result->voxel(pos) = static_cast<U>( gradientMagnitude * maxValueU );

            }
        }
    }
    return result;
}

template<class U>
VolumeHandle* calcGradientMagnitudes(const VolumeHandleBase* handle) {
    const Volume* input = handle->getRepresentation<Volume>();
    VolumeAtomic<U>* ret = 0;
    if (typeid(*input) == typeid(Volume3xUInt8))
        ret = calcGradientMagnitudesGeneric<U, tgt::Vector3<uint8_t> >(handle);
    else if (typeid(*input) == typeid(Volume3xUInt16))
        ret = calcGradientMagnitudesGeneric<U, tgt::Vector3<uint16_t> >(handle);
    else if (typeid(*input) == typeid(Volume4xUInt8))
        ret = calcGradientMagnitudesGeneric<U, tgt::Vector4<uint8_t> >(handle);
    else if (typeid(*input) == typeid(Volume4xUInt16))
        ret = calcGradientMagnitudesGeneric<U, tgt::Vector4<uint16_t> >(handle);
    else if (typeid(*input) == typeid(Volume3xFloat))
        ret = calcGradientMagnitudesGeneric<U, tgt::Vector3<float> >(handle);
    else if (typeid(*input) == typeid(Volume3xDouble))
        ret = calcGradientMagnitudesGeneric<U, tgt::Vector3<double> >(handle);
    else if (typeid(*input) == typeid(Volume4xFloat))
        ret = calcGradientMagnitudesGeneric<U, tgt::Vector4<float> >(handle);
    else if (typeid(*input) == typeid(Volume4xDouble))
        ret = calcGradientMagnitudesGeneric<U, tgt::Vector4<double> >(handle);
    else {
        LERRORC("calcGradientMagnitudes", "Unhandled type!");
    }
    return new VolumeHandle(ret, handle);
}

/**
 * Computes an simple approximation of the second directional derivative along the gradient direction at each voxel.
 * The calculation is done by applying the Laplacian operator.
 *
 * Use uint8_t or uint16_t as U template argument in order to generate 8 or 16 bit datasets.
 */
template<class U, class T>
VolumeAtomic<U>* calc2ndDerivatives(const VolumeAtomic<T> *input) {

    VolumeAtomic<U>* result = new VolumeAtomic<U>(input->getDimensions(), input->getSpacing());

    float maxValueT;
    if ( typeid(*input) == typeid(VolumeUInt8)  ||
         typeid(*input) == typeid(VolumeUInt16) ||
         typeid(*input) == typeid(VolumeUInt32) )
        maxValueT = static_cast<float>( (1 << input->getBitsStored()) - 1);
    else if ( typeid(*input) == typeid(VolumeFloat) || typeid(*input) == typeid(VolumeDouble))
        maxValueT = 1.f;
    else {
        LERRORC("calc2ndDerivatives", "Unknown or unsupported input volume type");
        tgtAssert(false, "Unknown or unsupported input volume type");
        return result;
    }

    float maxValueU;
    if ( typeid(*result) == typeid(VolumeUInt8)  ||
         typeid(*result) == typeid(VolumeUInt16) ||
         typeid(*result) == typeid(VolumeUInt32) )
        maxValueU = static_cast<float>( (1 << result->getBitsStored()) - 1);
    else if ( typeid(*result) == typeid(VolumeFloat) || typeid(*result) == typeid(VolumeDouble))
        maxValueU = 1.f;
    else {
        LERRORC("calc2ndDerivatives", "Unknown or unsupported output volume type");
        tgtAssert(false, "Unknown or unsupported output volume type");
        return result;
    }


    float derivative;
    ivec3 pos;
    ivec3 dim = input->getDimensions();
    for (pos.z = 0; pos.z < dim.z; pos.z++) {
        for (pos.y = 0; pos.y < dim.y; pos.y++) {
            for (pos.x = 0; pos.x < dim.x; pos.x++) {

                if (pos.x >= 1 && pos.x < dim.x-1 &&
                    pos.y >= 1 && pos.y < dim.y-1 &&
                    pos.z >= 1 && pos.z < dim.z-1)
                {
                    //left plane
                    //T v000 = input->voxel(pos + ivec3(-1,-1,-1));
                    //T v001 = input->voxel(pos + ivec3(-1, -1, 0));
                    //T v002 = input->voxel(pos + ivec3(-1, -1, 1));
                    //T v010 = input->voxel(pos + ivec3(-1, 0, -1));
                    T v011 = input->voxel(pos + ivec3(-1, 0, 0));
                    //T v012 = input->voxel(pos + ivec3(-1, 0, 1));
                    //T v020 = input->voxel(pos + ivec3(-1, 1, -1));
                    //T v021 = input->voxel(pos + ivec3(-1, 1, 0));
                    //T v022 = input->voxel(pos + ivec3(-1, 1, 1));

                    //mid plane
                    //T v100 = input->voxel(pos + ivec3(0, -1, -1));
                    T v101 = input->voxel(pos + ivec3(0, -1, 0));
                    //T v102 = input->voxel(pos + ivec3(0, -1, 1));
                    T v110 = input->voxel(pos + ivec3(0, 0, -1));
                    T v111 = input->voxel(pos + ivec3(0, 0, 0));
                    T v112 = input->voxel(pos + ivec3(0, 0, 1));
                    //T v120 = input->voxel(pos + ivec3(0, 1, -1));
                    T v121 = input->voxel(pos + ivec3(0, 1, 0));
                    //T v122 = input->voxel(pos + ivec3(0, 1, 1));

                    //right plane
                    //T v200 = input->voxel(pos + ivec3(1, -1, -1));
                    //T v201 = input->voxel(pos + ivec3(1, -1, 0));
                    //T v202 = input->voxel(pos + ivec3(1, -1, 1));
                    //T v210 = input->voxel(pos + ivec3(1, 0, -1));
                    T v211 = input->voxel(pos + ivec3(1, 0, 0));
                    //T v212 = input->voxel(pos + ivec3(1, 0, 1));
                    //T v220 = input->voxel(pos + ivec3(1, 1, -1));
                    //T v221 = input->voxel(pos + ivec3(1, 1, 0));
                    //T v222 = input->voxel(pos + ivec3(1, 1, 1));

                    // Original Laplace operator
                    derivative = static_cast<float>( -6.0*v111                  +
                                                      v101 + v110 + v112 + v121  +
                                                      v011 + v211     );

                    // Simple Laplacian of Gaussian
                    /*derivative = static_cast<double>( -36.0*v111 +
                                                      4.0*v101 + 4.0*v110 + 4.0*v112 + 4.0*v121  +
                                                      4.0*v011 + 4.0*v211 +
                                                      v100 + v102 + v120 + v122 +
                                                      v001 + v010 + v012 + v021 +
                                                      v201 + v210 + v212 + v221      ) / 8.0;  */

                }
                else {
                    derivative = 0.f;
                }

                // map value from [-maxT:maxT] to [0:maxU] since we expect an unsigned volume as input/output type
                result->voxel(pos) = static_cast<U>( ( (derivative / maxValueT) / 2.f + 0.5f ) * maxValueU );
            }
        }
    }
    return result;
}


/**
 * Calculates the curvature for each voxel.
 *
 * Use uint8_t or uint16_t as U template argument in order to generate 8 or 16 bit datasets.
 */
template<class U, class T>
VolumeHandle* calcCurvature(const VolumeHandleBase* handle, unsigned int curvatureType) {
    const VolumeAtomic<T>* input = dynamic_cast<const VolumeAtomic<T>*>(handle->getRepresentation<Volume>());
    VolumeAtomic<U>* result = new VolumeAtomic<U>(input->getDimensions());

    ivec3 pos;
    ivec3 dim = input->getDimensions();
    float minCurvature = std::numeric_limits<float>::max();
    float maxCurvature = std::numeric_limits<float>::min();
    for (pos.z = 0; pos.z < dim.z; pos.z++) {
        for (pos.y = 0; pos.y < dim.y; pos.y++) {
            for (pos.x = 0; pos.x < dim.x; pos.x++) {
                if (pos.x >= 2 && pos.x < dim.x-2 &&
                    pos.y >= 2 && pos.y < dim.y-2 &&
                    pos.z >= 2 && pos.z < dim.z-2)
                {

                    // fetch necessary data
                    float c = input->getVoxelFloat(pos);

                    float r0 = input->getVoxelFloat(pos+ivec3(1,0,0));
                    float r1 = input->getVoxelFloat(pos+ivec3(2,0,0));
                    float l0 = input->getVoxelFloat(pos+ivec3(-1,0,0));
                    float l1 = input->getVoxelFloat(pos+ivec3(-2,0,0));

                    float u0 = input->getVoxelFloat(pos+ivec3(0,1,0));
                    float u1 = input->getVoxelFloat(pos+ivec3(0,2,0));
                    float d0 = input->getVoxelFloat(pos+ivec3(0,-1,0));
                    float d1 = input->getVoxelFloat(pos+ivec3(0,-2,0));

                    float f0 = input->getVoxelFloat(pos+ivec3(0,0,1));
                    float f1 = input->getVoxelFloat(pos+ivec3(0,0,2));
                    float b0 = input->getVoxelFloat(pos+ivec3(0,0,-1));
                    float b1 = input->getVoxelFloat(pos+ivec3(0,0,-2));

                    float ur0 = input->getVoxelFloat(pos+ivec3(1,1,0));
                    float dr0 = input->getVoxelFloat(pos+ivec3(1,-1,0));
                    float ul0 = input->getVoxelFloat(pos+ivec3(-1,1,0));
                    float dl0 = input->getVoxelFloat(pos+ivec3(-1,-1,0));

                    float fr0 = input->getVoxelFloat(pos+ivec3(1,0,1));
                    float br0 = input->getVoxelFloat(pos+ivec3(1,0,-1));
                    float fl0 = input->getVoxelFloat(pos+ivec3(-1,0,1));
                    float bl0 = input->getVoxelFloat(pos+ivec3(-1,0,-1));

                    float uf0 = input->getVoxelFloat(pos+ivec3(0,1,1));
                    float ub0 = input->getVoxelFloat(pos+ivec3(0,1,-1));
                    float df0 = input->getVoxelFloat(pos+ivec3(0,-1,1));
                    float db0 = input->getVoxelFloat(pos+ivec3(0,-1,-1));

                    vec3 gradient = vec3(l0-r0,d0-u0,b0-f0);

                    float gradientLength = length(gradient);
                    if (gradientLength == 0.0f) gradientLength = 1.0f;

                    vec3 n = -gradient / gradientLength;

                    tgt::mat3 nxn; // matrix to hold the outer product of n and n^T
                    for (int i=0; i<3; ++i)
                        for (int j=0; j<3; ++j)
                            nxn[i][j] = n[i]*n[j];

                    tgt::mat3 P = tgt::mat3::identity - nxn;

                    // generate Hessian matrix
                    float fxx = (((r1-c)/2.0f)-((c-l1)/2.0f))/2.0f;
                    float fyy = (((u1-c)/2.0f)-((c-d1)/2.0f))/2.0f;
                    float fzz = (((f1-c)/2.0f)-((c-b1)/2.0f))/2.0f;
                    float fxy = (((ur0-ul0)/2.0f)-((dr0-dl0)/2.0f))/2.0f;
                    float fxz = (((fr0-fl0)/2.0f)-((br0-bl0)/2.0f))/2.0f;
                    float fyz = (((uf0-ub0)/2.0f)-((df0-db0)/2.0f))/2.0f;
                    tgt::mat3 H;
                    H[0][0] = fxx;
                    H[0][1] = fxy;
                    H[0][2] = fxz;
                    H[1][0] = fxy;
                    H[1][1] = fyy;
                    H[1][2] = fyz;
                    H[2][0] = fxz;
                    H[2][1] = fyz;
                    H[2][2] = fzz;

                    tgt::mat3 G = -P*H*P / gradientLength;

                    // compute trace of G
                    float trace = G.t00 + G.t11 + G.t22;

                    // compute Frobenius norm of G
                    float F = 0.0f;
                    for (int i=0; i<3; ++i)
                        for (int j=0; j<3; ++j)
                            F += powf(std::abs(G[i][j]), 2.0f);
                    F = sqrt(F);

                    float kappa1 = (trace + sqrtf(2.0f * powf(F,2.0f) - powf(trace,2.0f))) / 2.0f;
                    float kappa2 = (trace - sqrtf(2.0f * powf(F,2.0f) - powf(trace, 2.0f))) / 2.0f;

                    float curvature = 0.f;
                    if (curvatureType == 0) // first principle
                        curvature = kappa1;
                    else if (curvatureType == 1) // second principle
                        curvature = kappa2;
                    else if (curvatureType == 2) // mean
                        curvature = (kappa1+kappa2)/2.0f;
                    else if (curvatureType == 3) // Gaussian
                        curvature = kappa1*kappa2;
                    result->voxel(pos) = static_cast<U>(curvature);

                    if (curvature < minCurvature) minCurvature = curvature;
                    else if (curvature > maxCurvature) maxCurvature = curvature;
                } else
                    result->voxel(pos) = static_cast<U>(0.0f);
            }
        }
    }

    // scale curvature to lie in interval [0.0,1.0], where 0.5 equals zero curvature
    for (pos.z = 0; pos.z < dim.z; pos.z++) {
        for (pos.y = 0; pos.y < dim.y; pos.y++) {
            for (pos.x = 0; pos.x < dim.x; pos.x++) {
                float c = result->getVoxelFloat(pos, 0);
                if (c < 0.0f) c /= -minCurvature;
                else if (c >= 0.0f) c /= maxCurvature;
                c /= 2.0f;
                c += 0.5f;
                result->voxel(pos) = static_cast<U>(c);
            }
        }
    }
    return new VolumeHandle(result, handle);
}


/**
 * Calculates curvature.
 *
 * Use uint8_t or col4 as template argument in order to generate 8 or 4x8 bit datasets.
 * Use uint16_t or Vector4<uint16_t> as template argument in order
 * to generate 16 or 4x16 bit datasets.
 */
template<class U>
VolumeHandle* calcCurvature(const VolumeHandleBase* handle, unsigned int curvatureType) {
    const Volume* vol = handle->getRepresentation<Volume>();
    if (vol->getNumChannels() != 1) {
        LERRORC("calcCurvature", "calcCurvature needs an input volume with 1 intensity channel");
        return 0;
    }

    if (vol->getBitsStored() == 8) {
        return calcCurvature<U, uint8_t>(handle, curvatureType);
    }
    else if (vol->getBitsStored() == 12) {
        return calcCurvature<U, uint16_t>(handle, curvatureType);
    }
    else if (vol->getBitsStored() == 16) {
        return calcCurvature<U, uint16_t>(handle, curvatureType);
    }
    LERRORC("calcCurvature", "calcCurvature needs a 8-, 12- or 16-bit dataset as input");
    return 0;

}


/**
 * Calculates gradients with neighborhood of 26.
 * Slower, but should result in better gradients.
 */
VRN_CORE_API VolumeHandle* calcGradients26(const VolumeHandleBase* vol);

/**
 * Filters gradients stored in a 32 bit volume data set. A very simple interpolation
 * adapted to binary data sets is performed, which replaces zero gradients by
 * non-zero gradients in the direct vicinity.
 *
 * @param vol 32 bit volume data set with stored gradients.
 */
VRN_CORE_API VolumeHandle* filterGradients(const VolumeHandleBase* vol);

/**
 * Filters gradients stored in a 32 bit volume data set.
 * The mid value of gradients in vicinity is saved.
 *
 * @param vol 32 bit volume data set with stored gradients.
 */
VRN_CORE_API VolumeHandle* filterGradientsMid(const VolumeHandleBase* vol);

/**
 * Filters gradients stored in a 32 bit volume data set.
 * gradient values in vicinity are weighted.
 *
 * @param vol 32 bit volume data set with stored gradients.
 */
VRN_CORE_API VolumeHandle* filterGradientsWeighted(const VolumeHandleBase* vol, bool intensityCheck);

} // namespace

#endif //VRN_GRADIENT_H
