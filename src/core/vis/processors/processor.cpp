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

#include "voreen/core/vis/processors/processor.h"

#include "tgt/glmath.h"
#include "tgt/camera.h"
#include "tgt/shadermanager.h"

#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/processors/networkevaluator.h"
#include "voreen/core/vis/lightmaterial.h"

#include <sstream>

using tgt::vec3;
using tgt::vec4;
using tgt::Color;

namespace voreen {

HasShader::HasShader()
    : needRecompileShader_(true)
{
}

HasShader::~HasShader() {
}

void HasShader::compileShader() {
    if (needRecompileShader_) {
        compile();
        needRecompileShader_ = false;
    }
}

void HasShader::invalidateShader() {
    needRecompileShader_ = true;
}

//---------------------------------------------------------------------------

const Identifier Processor::setBackgroundColor_("set.backgroundColor");

const std::string Processor::loggerCat_("voreen.Processor");

const std::string Processor::XmlElementName_("Processor");

Processor::Processor(tgt::Camera* camera, TextureContainer* tc)
    : MessageReceiver()
    , Serializable()
    , cacheable_(true),
    useVolumeCaching_(true)
    , isCoprocessor_(false)
    , camera_(camera)
    , tc_(tc)
    , tm_()
    , backgroundColor_(setBackgroundColor_, "Background color", tgt::Color(0.0f, 0.0f, 0.0f, 0.0f))
    , lightPosition_(LightMaterial::setLightPosition_, "Light source position", tgt::vec4(2.3f, 1.5f, 1.5f, 1.f),
                     tgt::vec4(-10), tgt::vec4(10))
    , lightAmbient_(LightMaterial::setLightAmbient_, "Ambient light", tgt::Color(0.4f, 0.4f, 0.4f, 1.f))
    , lightDiffuse_(LightMaterial::setLightDiffuse_, "Diffuse light", tgt::Color(0.8f, 0.8f, 0.8f, 1.f))
    , lightSpecular_(LightMaterial::setLightSpecular_, "Specular light", tgt::Color(0.6f, 0.6f, 0.6f, 1.f))
    , lightAttenuation_(LightMaterial::setLightAttenuation_, "Attenutation", tgt::vec3(1.f, 0.f, 0.f))
    , materialAmbient_(LightMaterial::setMaterialAmbient_, "Ambient material color", tgt::Color(1.f, 1.f, 1.f, 1.f))
    , materialDiffuse_(LightMaterial::setMaterialDiffuse_, "Diffuse material color", tgt::Color(1.f, 1.f, 1.f, 1.f))
    , materialSpecular_(LightMaterial::setMaterialSpecular_, "Specular material color", tgt::Color(1.f, 1.f, 1.f, 1.f))
    , materialEmission_(LightMaterial::setMaterialEmission_, "Emissive material color", tgt::Color(0.f, 0.f, 0.f, 1.f))
    , materialShininess_(LightMaterial::setMaterialShininess_, "Shininess", 60.f, 0.1f, 128.f)
    , initStatus_(VRN_OK)
{
    addProperty(&lightPosition_);
    lightPosition_.setVisible(false);
}

Processor::~Processor() {
    for (size_t i=0; i<inports_.size(); ++i)
        delete inports_.at(i);
    for (size_t i=0; i<outports_.size(); ++i)
        delete outports_.at(i);
    for (size_t i=0; i<coProcessorInports_.size(); ++i)
        delete coProcessorInports_.at(i);
    for (size_t i=0; i<coProcessorOutports_.size(); ++i)
        delete coProcessorOutports_.at(i);
    for (size_t i=0; i<privatePorts_.size(); ++i)
        delete privatePorts_.at(i);
}

int Processor::initializeGL() {
    return VRN_OK;
}

Message* Processor::call(Identifier /*ident*/, LocalPortMapping*) {
    return 0;
}

void Processor::addProperty(Property* prop) {
    props_.push_back(prop);
    prop->setOwner(this);
}

const Properties& Processor::getProperties() const {
    return props_;
}

bool Processor::connect(Port* outport, Port* inport) {
    if (!inport->isOutport()) {
        if ( (outport->getType().getSubString(0) == inport->getType().getSubString(0)) && (inport->getProcessor() != this) ) {
            if (inport->allowMultipleConnections() || inport->getConnected().size() == 0 ) {
                if (!inport->isConnectedTo(outport)) {
                    outport->addConnection(inport);
                    inport->addConnection(outport);
                    return true;
                }
            }
        }
    }
    return false;
}

bool Processor::testConnect(Port* outport, Port* inport) {
    if (!inport->isOutport()) {
        if ( (outport->getType().getSubString(0) == inport->getType().getSubString(0)) && (inport->getProcessor() != this) ) {
            if (inport->allowMultipleConnections() || inport->getConnected().size() == 0 ) {
                if (!inport->isConnectedTo(outport)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Processor::disconnect(Port* outport, Port* inport) {
    bool success = false;
    for (size_t i=0; i<inport->getConnected().size(); ++i) {
        if (inport->getConnected()[i] == outport) {
            inport->getConnected().erase(inport->getConnected().begin() + i);
            success = true;
        }
    }
    if (!success)
        return false;

    for (size_t i=0; i<outport->getConnected().size(); ++i) {
        if (outport->getConnected()[i] == inport) {
            outport->getConnected().erase(outport->getConnected().begin() + i);
            return true;
        }
    }

    return false;
}

Port* Processor::getPort(Identifier ident) {
    for (size_t i=0; i <inports_.size(); ++i) {
        if (inports_.at(i)->getType() == ident)
            return inports_.at(i);
    }

    for (size_t i=0; i <outports_.size(); ++i) {
        if (outports_.at(i)->getType() == ident)
            return outports_.at(i);
    }

    for (size_t i=0; i <coProcessorInports_.size(); ++i) {
        if (coProcessorInports_.at(i)->getType() == ident)
            return coProcessorInports_.at(i);
    }

    for (size_t i=0; i <coProcessorOutports_.size(); ++i) {
        if (coProcessorOutports_.at(i)->getType() == ident)
            return coProcessorOutports_.at(i);
    }

    for (size_t i=0; i<privatePorts_.size(); ++i) {
        if (privatePorts_.at(i)->getType() == ident)
            return privatePorts_.at(i);
    }

    return 0;
}

void Processor::createInport(Identifier ident, bool allowMultipleConnectios) {
    inports_.push_back(new Port(ident,this,false,allowMultipleConnectios,false));
}

void Processor::createOutport(Identifier ident, bool isPersistent,Identifier inport) {
    Port* newOutport = new Port(ident,this,true,true,isPersistent);
    outports_.push_back(newOutport);
    if (inport != "dummy.port.unused") {
        if (getPort(inport) != 0)
            outportToInportMap_.insert(std::pair<Port*,Port*>(newOutport,getPort(inport)));
        else
            LWARNING("Couldn't find the inport in Processor::createOutport(Identifier ident, Identifier ident)");
    }
}

void Processor::createCoProcessorInport(Identifier ident, bool allowMultipleConnectios) {
    coProcessorInports_.push_back(new Port(ident,this,false,allowMultipleConnectios,false));
}

void Processor::createCoProcessorOutport(Identifier ident, FunctionPointer function, bool allowMultipleConnectios) {
    Port* newPort = new Port(ident,this,true,allowMultipleConnectios,false);
    newPort->setFunctionPointer(function);
    coProcessorOutports_.push_back(newPort);
}

void Processor::createPrivatePort(Identifier ident) {
    privatePorts_.push_back(new Port(ident,this,true,false,true));
}

void Processor::processMessage(Message* msg, const Identifier& dest) {
    MessageReceiver::processMessage(msg, dest);
    if (msg->id_ == setBackgroundColor_) {
        backgroundColor_.set(msg->getValue<tgt::Color>());
    }
    else if (msg->id_ == "set.viewport") {
        if (camera_) {
            tgt::ivec2 v = msg->getValue<tgt::ivec2>();
            glViewport(0, 0, v.x, v.y);
            setSizeTiled(v.x, v.y);
            if (tc_)
                tc_->setSize(v);

            LINFO("set.viewport to " << v);
        }
    }
    else if (msg->id_ == LightMaterial::setLightPosition_) {
        lightPosition_.set(msg->getValue<vec4>());
    }
    else if (msg->id_ == LightMaterial::setLightAmbient_) {
        lightAmbient_.set(msg->getValue<Color>());
    }
    else if (msg->id_ == LightMaterial::setLightDiffuse_) {
        lightDiffuse_.set(msg->getValue<Color>());
    }
    else if (msg->id_ == LightMaterial::setLightSpecular_) {
        lightSpecular_.set(msg->getValue<Color>());
    }
    else if (msg->id_ == LightMaterial::setLightAttenuation_) {
        lightAttenuation_.set(msg->getValue<vec3>());
    }
    else if (msg->id_ == LightMaterial::setMaterialAmbient_) {
        materialAmbient_.set(msg->getValue<Color>());
    }
    else if (msg->id_ == LightMaterial::setMaterialDiffuse_) {
        materialDiffuse_.set(msg->getValue<Color>());
    }
    else if (msg->id_ == LightMaterial::setMaterialSpecular_) {
        materialSpecular_.set(msg->getValue<Color>());
    }
    else if (msg->id_ == LightMaterial::setMaterialEmission_) {
        materialEmission_.set(msg->getValue<Color>());
    }
    else if (msg->id_ == LightMaterial::setMaterialShininess_) {
        materialShininess_.set(msg->getValue<float>());
    }
}

void Processor::setTextureContainer(TextureContainer* tc) {
    tc_ = tc;
}

void Processor::setCamera(tgt::Camera* camera) {
    camera_ = camera;
}

tgt::Camera* Processor::getCamera() const {
    return camera_;
}

TextureContainer* Processor::getTextureContainer() {
    return tc_;
}

void Processor::setGeometryContainer(GeometryContainer* geoCont) {
    geoContainer_ = geoCont;
}

GeometryContainer* Processor::getGeometryContainer() const {
    return geoContainer_;
}

void Processor::setName(const std::string& name) {
    name_ = name;
}

std::string Processor::getName() const {
    return name_;
}

const std::string Processor::getProcessorInfo() const {
    return "No information available";
}

std::vector<Port*> Processor::getInports() const {
    return inports_;
}

std::vector<Port*> Processor::getOutports() const {
    return outports_;
}

std::vector<Port*> Processor::getCoProcessorInports() const {
    return coProcessorInports_;
}

std::vector<Port*> Processor::getCoProcessorOutports() const {
    return coProcessorOutports_;
}

std::vector<Port*> Processor::getPrivatePorts() const {
    return privatePorts_;
}

Port* Processor::getInport(Identifier type) {
    for (size_t i=0; i < inports_.size(); ++i) {
        if (inports_.at(i)->getType() == type)
            return inports_.at(i);
    }
    return 0;
}

Port* Processor::getOutport(Identifier type) {
    for (size_t i=0; i < outports_.size(); ++i) {
        if (outports_.at(i)->getType() == type)
            return outports_.at(i);
    }
    return 0;
}

void Processor::setSize(const tgt::ivec2 &size) {
    setSize(static_cast<tgt::vec2>(size));
}

void Processor::setSize(const tgt::vec2& size) {
    if (camera_ && size_ != size) {
        size_ = size;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        camera_->getFrustum().setRatio(size.x/size.y);
        camera_->updateFrustum();
        tgt::loadMatrix(camera_->getProjectionMatrix());

        glMatrixMode(GL_MODELVIEW);

        invalidate();
    }
}

tgt::ivec2 Processor::getSize() const {
    return static_cast<tgt::ivec2>(size_);
}

tgt::vec2 Processor::getSizeFloat() const {
    return size_;
}

void Processor::invalidate() {
    MsgDistr.postMessage(new ProcessorPointerMsg(NetworkEvaluator::processorInvalidated_, this));
}

void Processor::renderQuad() {
    glBegin(GL_QUADS);
        glVertex2f(-1.0, -1.0);
        glVertex2f( 1.0, -1.0);
        glVertex2f( 1.0,  1.0);
        glVertex2f(-1.0,  1.0);
    glEnd();
}

std::string Processor::generateHeader() {
    std::string header = "";

    // #version needs to be on the very first line for some compilers (ATI)
    header += "#version 120\n";

    if (tc_) {
        if (tc_->getTextureContainerTextureType() == TextureContainer::VRN_TEXTURE_2D)
            header += "#define VRN_TEXTURE_2D\n";
        else if (tc_->getTextureContainerTextureType() == TextureContainer::VRN_TEXTURE_RECTANGLE)
            header += "#define VRN_TEXTURE_RECTANGLE\n";
    }

    if (GLEW_NV_fragment_program2) {
        GLint i = -1;
        glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_LOOP_COUNT_NV, &i);
        if (i > 0) {
            std::ostringstream o;
            o << i;
            header += "#define VRN_MAX_PROGRAM_LOOP_COUNT " + o.str() + "\n";
        }
    }

    return header;
}

// Parameters currently set:
// - screenDim_
// - screenDimRCP_
// - cameraPosition_ (camera position in world coordinates)
// - lightPosition_ (light source position in world coordinates)
void Processor::setGlobalShaderParameters(tgt::Shader* shader) {
    shader->setIgnoreUniformLocationError(true);

    if (tc_) {
        shader->setUniform("screenDim_", tgt::vec2(tc_->getSize()));
        shader->setUniform("screenDimRCP_", 1.f / tgt::vec2(tc_->getSize()));
    }

    // camera position in world coordinates
    shader->setUniform("cameraPosition_", camera_->getPosition());

    // light source position in world coordinates
    shader->setUniform("lightPosition_", getLightPosition());

    shader->setIgnoreUniformLocationError(false);
}

void Processor::setLightingParameters() {
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient_.get().elem );
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse_.get().elem );
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular_.get().elem );
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, lightAttenuation_.get().x );
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, lightAttenuation_.get().y );
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, lightAttenuation_.get().z );

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, materialEmission_.get().elem);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess_.get());
}

void Processor::setSizeTiled(uint width, uint height) {
    setSize(tgt::ivec2(width, height));

    // gluPerspective replacement taken from
    // http://nehe.gamedev.net/data/articles/article.asp?article=11

    float fovY = 45.f;
    float aspect = static_cast<float>(width) / height;
    float zNear = 0.1f;
    float zFar = 50.f;

    float fw, fh;
    fh = tanf(fovY / 360 * tgt::PIf) * zNear;
    fw = fh * aspect;

    tgt::Frustum frust_ = tgt::Frustum(-fw, fw, - fh, fh, zNear, zFar);
    camera_->setFrustum(frust_);
    camera_->updateFrustum();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    tgt::loadMatrix(camera_->getProjectionMatrix());
    glMatrixMode(GL_MODELVIEW);
}

bool Processor::isEndProcessor() const {
    return (outports_.size() == 0 && coProcessorOutports_.size() == 0);
}

std::string Processor::getState() const {
    std::string state;
    for (size_t i = 0; i < props_.size(); ++i)
        state += props_[i]->toString();
    return state;
}

std::string Processor::getXmlElementName() const {
    return XmlElementName_;
}

const Identifier Processor::getClassName(TiXmlElement* processorElem) {
    if (!processorElem)
        throw XmlElementException("Can't get ClassName of Null-Pointer!");

    if (processorElem->Value() != XmlElementName_)
        throw XmlElementException("Can't get ClassName of a " + std::string(processorElem->Value())
                                  + " - need " + XmlElementName_ + "!");

    if (!processorElem->Attribute("type"))
        throw XmlAttributeException("Needed attribute 'type' missing");


    return Identifier(processorElem->Attribute("type"));
}

TiXmlElement* Processor::serializeToXml() const {
    TiXmlElement* processorElem = new TiXmlElement(XmlElementName_);

    // metadata
    TiXmlElement* metaElem = meta_.serializeToXml();
    processorElem->LinkEndChild(metaElem);

    // misc settings
    processorElem->SetAttribute("type", getClassName().getName());
    processorElem->SetAttribute("name", getName());

    // serialize the properties of the processor
    std::vector<Property*> properties = getProperties();
    for (size_t i = 0; i < properties.size(); ++i) {
        if (properties[i]->isSerializable() || ignoreIsSerializable()) {
            TiXmlElement* propElem = properties[i]->serializeToXml();
            processorElem->LinkEndChild(propElem);
        }
    }

    return processorElem;
}

TiXmlElement* Processor::serializeToXml(const std::map<Processor*,int> idMap) const {
    TiXmlElement* processorElem = serializeToXml();
    Processor* self = const_cast<Processor*>(this); // For const-correctness - can't use 'this' in Map::find
    processorElem->SetAttribute("id", idMap.find(self)->second);
    // serialize the (in)ports a.k.a. connection info
    std::vector<Port*> outports = getOutports();
    std::vector<Port*> coprocessoroutports = getCoProcessorOutports();
    // append coprocessoroutports to outports because we can handle them identically
    outports.insert(outports.end(), coprocessoroutports.begin(), coprocessoroutports.end());
    for (size_t i=0; i < outports.size(); ++i) {
        // add (out)port
        TiXmlElement* outportElem = new TiXmlElement("Outport");
        outportElem->SetAttribute("type", outports[i]->getType().getName());
        processorElem->LinkEndChild(outportElem);
        // add processors connected to this (out)port via one of their (in)ports
        std::vector<Port*> connectedPorts = outports[i]->getConnected();
        for (size_t j=0; j < connectedPorts.size(); ++j) {
            TiXmlElement* connectedProcessorElem = new TiXmlElement(XmlElementName_);
            connectedProcessorElem->SetAttribute("id", idMap.find(connectedPorts[j]->getProcessor())->second);
            connectedProcessorElem->SetAttribute("port", connectedPorts[j]->getType().getName());
            // For inports that allow multiple connections
            // need to know the index of outport in the inports connected ports
            if (connectedPorts[j]->allowMultipleConnections())
                connectedProcessorElem->SetAttribute("order", connectedPorts[j]->getIndexOf(outports[i]));
            outportElem->LinkEndChild(connectedProcessorElem);
        }
    }

    return processorElem;
}

void Processor::updateFromXml(TiXmlElement* processorElem) {
    errors_.clear();
    // meta
    TiXmlElement* metaElem = processorElem->FirstChildElement(meta_.getXmlElementName());
    if (metaElem) {
        meta_.updateFromXml(metaElem);
        errors_.store(meta_.errors());
    }
    else
        errors_.store(XmlElementException("Metadata missing!")); // TODO better exception
    // read properties
    if (processorElem->Attribute("name"))
        setName(processorElem->Attribute("name"));

    for (TiXmlElement* propertyElem = processorElem->FirstChildElement(Property::XmlElementName_);
        propertyElem != 0;
        propertyElem = propertyElem->NextSiblingElement(Property::XmlElementName_))
    {
        for (size_t j = 0; j < props_.size(); ++j) {
            try {
                if (props_[j]->getIdent() == Property::getIdent(propertyElem) ) {
                    props_[j]->updateFromXml(propertyElem);
                    errors_.store(props_[j]->errors());
                }
            } catch (SerializerException& e) {
                errors_.store(e);
            }
        }
    }
}

std::pair<int, Processor::ConnectionMap> Processor::getMapAndUpdateFromXml(TiXmlElement* processorElem) {
    updateFromXml(processorElem);
    ConnectionSide out, in;
    ConnectionMap lcm;
    int id;
    if (processorElem->QueryIntAttribute("id", &id) != TIXML_SUCCESS) {
        errors_.store(XmlAttributeException("Required attribute 'id' of Processor missing!"));
        id = -1;
    }
    else {
        out.processorId = id;

        // read outports
        TiXmlElement* outportElem;
        for (outportElem = processorElem->FirstChildElement("Outport");
            outportElem;
            outportElem = outportElem->NextSiblingElement("Outport"))
        {
            if (!outportElem->Attribute("type")) {
                errors_.store(XmlAttributeException("Required attribute 'type' of Port missing!"));
            }
            else {
                out.portId = outportElem->Attribute("type");
                //if (outportElem->QueryIntAttribute("order", &out.order) != TIXML_SUCCESS)
                    out.order = 0; // Annotation: The order for the first Port is not really needed
                // read processors connected to outport
                TiXmlElement* connectedprocessorElem;
                for (connectedprocessorElem = outportElem->FirstChildElement(XmlElementName_);
                    connectedprocessorElem;
                    connectedprocessorElem = connectedprocessorElem->NextSiblingElement(XmlElementName_))
                {
                    try {
                        if (connectedprocessorElem->QueryIntAttribute("id", &in.processorId) != TIXML_SUCCESS)
                            throw XmlAttributeException("Required attribute id of Processor missing!");
                        if (!connectedprocessorElem->Attribute("port"))
                            throw XmlAttributeException("Required attribute port of Processor missing!");

                        if (connectedprocessorElem->QueryIntAttribute("order", &in.order) != TIXML_SUCCESS)
                            in.order = 0;
                        in.portId = connectedprocessorElem->Attribute("port");

                        // store this connection in the map
                        lcm.push_back(std::make_pair(out, in));
                    }
                    catch (SerializerException& e) {
                        errors_.store(e);
                    }
                }
            }
        }
    }

    return std::make_pair(id, lcm);
}

void Processor::addToMeta(TiXmlElement* elem) {
    meta_.addData(elem);
}

void Processor::removeFromMeta(std::string elemName) {
    meta_.removeData(elemName);
}

void Processor::clearMeta() {
    meta_.clearData();
}

TiXmlElement* Processor::getFromMeta(std::string elemName) const {
    return meta_.getData(elemName);
}

std::vector<TiXmlElement*> Processor::getAllFromMeta() const {
    return meta_.getAllData();
}

bool Processor::hasInMeta(std::string elemName) const {
    return meta_.hasData(elemName);
}

tgt::vec3 Processor::getLightPosition() const {
    return tgt::vec3(lightPosition_.get().x, lightPosition_.get().y, lightPosition_.get().z);
}

bool Processor::getIsCoprocessor() const {
    return isCoprocessor_;
}

void Processor::setIsCoprocessor(bool b) {
    isCoprocessor_ = b;
}

bool Processor::getCacheable() const {
    return cacheable_;
}

void Processor::setCacheable(bool b) {
    cacheable_ = b;
}

int Processor::getInitStatus() const {
    return initStatus_;
}

std::map<Port*,Port*> Processor::getOutportToInportMap() {
    return outportToInportMap_;
}

} // namespace voreen
