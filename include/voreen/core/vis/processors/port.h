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

#ifndef VRN_PORT_H
#define VRN_PORT_H

#include "voreen/core/vis/message.h"

#include <vector>
#include <typeinfo>

namespace voreen {

class LocalPortMapping;
class Processor;

/**
 * This typedef defines a function pointer with an Identifier as parameter and a Message*
 * as return type. This is used in the CoProcessor ports. CoProcessors add a function pointer
 * to their CoProcessorOutports and by that define their functionality.
 */
typedef Message*(Processor::*FunctionPointer)(Identifier, LocalPortMapping*);

/**
 * This class is used in the portMapping process in the NetworkEvaluator. Every port of every processor
 * is mapped to an PortData Object or an object inheriting from PortData. This way the ports can be mapped
 * to any class as long as it inherits from PortData
 */
class PortData {
public:
    PortData(Identifier type);
    virtual ~PortData() {}

    /**
     * Gets the type. Used to differentiate between different classes all inheriting from PortData
     */
    Identifier getType() const { return type_; }

    /**
     * Sets the type. Used to differentiate between different classes all inheriting from PortData
     */
    void setType(Identifier type) { type_ = type; }

protected:
    /**
     * Used to differentiate between different classes all inheriting from PortData
     */
    Identifier type_;
};

//------------------------------------------------------------------------------

/**
 * PortData class for arbitrary content. Used for passing pointers through the network.
 */
template<class T>
class PortDataGeneric : public PortData {
public:
    /**
     * Ctor setting the PortData's identifier and data.
     * The identifier is composed as follows:
     *
     * std::string("portdata.") + typeid(T).name()
     *
     */
    PortDataGeneric(const T& data, const Identifier& name = std::string("portdata.") + typeid(T).name())
        : PortData(name), data_(data) {}

    /**
     * Dtor, does nothing.
     */
    virtual ~PortDataGeneric() {}

    void setData(const T& data) { data_ = data; }
    T getData() { return data_; }

protected:
    T data_;
};

typedef PortDataGeneric<int> PortDataTexture;
typedef PortDataGeneric<int> PortDataGeometry;

//------------------------------------------------------------------------------

/**
 * Stores a function pointer, pointing to a function in a processor.
 */
class PortDataCoProcessor : public PortDataGeneric<FunctionPointer> {
public:
    /**
     * Creates a PortDataCoProcessor with a functionPointer pointing to a function in
     * the given processor.
     */
    PortDataCoProcessor(Processor* processor, FunctionPointer functionPointer );
    virtual ~PortDataCoProcessor() {}

    /**
     * Gets the function pointer stored in this object
     */
    FunctionPointer getFunction() {return getData(); }

    /**
     * Sets the function pointer stored in this object
     */
    void setFunction(FunctionPointer function) { setData(function); }

    /**
     * Sets the processor which holds the function the function pointer points to.
     */
    void setProcessor(Processor* p) { processor_=p; }

    /**
     * Gets the processor which holds the function the function pointer points to.
     */
    Processor* getProcessor() { return processor_; }

    /**
     * Calls the function pointer. That can be done manually aswell, this is just for convenience.
     */
    Message* call(Identifier, LocalPortMapping* = 0);

protected:
    /**
     * The processor which holds the function the function pointer points to.
     */
    Processor* processor_;
};

//------------------------------------------------------------------------------

/**
 * This class describes a port of a Processor. Processors are connected
 * by their ports.
 */
class Port {
public:

    /**
     * Constructor.
     *@param processor The processor this port belongs to.
     *@param isOutport Is this Port an outgoing port or an ingoing?
     *@param allowMultipleConnections Can this port handle multiple connections? (Outports always can)
     *@param isPersistent If this is an outport and points to a TextureContainer target, shall this target
     *be persistent? (Not sure if this works yet! (Stephan) )
     */
    Port(Identifier type,
         Processor* processor,
         bool isOutport,
         bool allowMultipleConnections = false,
         bool isPersistent = false
        );

    // added by (df) for GenericPort (expermintal)
    virtual ~Port() {}

    /**
     * Returns all the ports that are connected to this port.
     */
    std::vector<Port*>& getConnected();

    /**
     * Returns the processor this port belongs to.
     */
    Processor* getProcessor() { return processor_; }

    /**
     * A port can hold a function pointer which the NetworkEvaluator reads and the puts into the
     * PortMapping of all the connected ports. This returns that function pointer.
     */
    FunctionPointer getFunctionPointer() { return function_; }

    /**
     * A port can hold a function pointer which the NetworkEvaluator reads and the puts into the
     * PortMapping of all the connected ports. This sets that function pointer.
     */
    void setFunctionPointer(FunctionPointer function) { function_ = function; }

    /**
     * Returns whether this port is an outport or not.
     */
    bool isOutport() const { return isOutport_; }

    /**
     * Returns whether this port allows multiple connections or not.
     */
    bool allowMultipleConnections() const { return allowMultipleConnections_; }

    /**
     * Returns whether this port is connected to the one given as parameter.
     */
    bool isConnectedTo(Port* port) const;

    /**
     * Returns the index of the given port in the vector of connected ports
     */
    int getIndexOf(Port* port) const;

    /**
     * Sets the type of this port. Something like "image.entrypoints" or "volume.dataset"
     */
    void setType(Identifier newType);

    /**
     * Gets the type of this port. Something like "image.entrypoints" or "volume.dataset"
     */
    Identifier getType() const;

    /**
     * Returns whether the result of the processor (passed on by this port) shall be persistent
     */
    bool getIsPersistent() { return isPersistent_; }

    /**
     * Sets whether the result of the  processor (passed on by this port) shall be persistent
     */
    void setIsPersistent(bool b) { isPersistent_ = b; }

    /**
     * Adds a new connection to the supplied port
     */
    void addConnection(Port* port);

    /**
     *Checks if this port is compatible to another port of the given type.
     */
    bool isCompatible(Identifier type);

    /**
     * Prints the processors this port is connected to. For debugging purposes.
     */
    std::string printConnections();

    int getData() { return data_; }     // FIXME: obsolete? (df)
    void setData(int i) { data_ = i; }  // FIXME: obsolete? (df)

    virtual PortData* getPortData() { return 0; }

private:

    /**
     * This can be used to pass numerous kinds of data from one port to another, like Volume numbers in a
     * VolumeContainer for example.
     */
    int data_;  // FIXME: obsolete? (df)

    /**
     * The type of this port. Something like "image.entrypoints" or "volume.dataset"
     */
    Identifier type_;

    /**
     * The ports connected to this one
     */
    std::vector<Port*> connectedPorts_;

    /**
     * The processor this port belongs to
     */
    Processor* processor_;

    /**
     * Is this port an outport or not
     */
    bool isOutport_;

    /**
     * Is this port allowed to have multiple connections?
     */
    bool allowMultipleConnections_;

    /**
     * A port can hold a function pointer which the NetworkEvaluator reads and the puts into the
     * PortMapping of all the connected ports. This is that function pointer.
     */
    FunctionPointer function_;

    /**
     * Sets whether the result of the ports processor (passed on by this port) shall be persistent
     */
    bool isPersistent_;
};

//------------------------------------------------------------------------------

template<typename T>
class GenericOutPort : public Port {
public:
    /**
     * Constructor.
     * @param   processor   The processor this port belongs to.
     * @param   isPersistent    If this is an outport and points to a TextureContainer target,
     *                          shall this target be persistent? (Not sure if this works yet! (Stephan))
     */
    GenericOutPort(const Identifier& type, T data, Processor* const processor,
        const bool isPersistent = false)
        : Port(type, processor, true, true, isPersistent),
        genericData_(data)
    {
    }

    virtual ~GenericOutPort() {}

    void setPortData(const T& data) { genericData_->setData(data); }
    virtual PortData* getPortData() { return &genericData_; }

private:
    PortDataGeneric<T> genericData_;
};

} // namespace voreen

#endif // VRN_PORT_H
