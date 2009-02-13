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

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
#include <GL/glut.h>

#include "tgt/init.h"
#include "tgt/glut/glutcanvas.h"
#include "tgt/navigation/trackball.h"
#include "tgt/navigation/navigation.h"
#include "tgt/shadermanager.h"
#include "tgt/camera.h"

#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/vis/trackballnavigation.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/processors/processorfactory.h"
#include "voreen/core/vis/processors/networkevaluator.h"
#include "voreen/core/vis/processors/networkserializer.h"
#include "voreen/core/vis/transfunc/transfuncintensity.h"
#include "voreen/core/volume/volumeset.h"
#include "voreen/core/volume/volumesetcontainer.h"

using namespace voreen;

//---------------------------------------------------------------------------

tgt::GLUTCanvas* canvas_ = 0;
GLfloat threshold_ = 0.1f;

//---------------------------------------------------------------------------

void init() {
    // initialize OpenGL part of tgt
    tgt::initGL();  
    // add shader path to the shader manager
    ShdrMgr.addPath("../../src/core/vis/glsl");

    // initialize the MessageDistributor
    tgt::Singleton<MessageDistributor>::init(new MessageDistributor());
	

	// init camera and connect it to the canvas
    tgt::Camera* camera_ = new tgt::Camera(tgt::vec3(0.0f, 0.0f, 3.75f),
                                           tgt::vec3(0.0f, 0.0f, 0.0f),
                                           tgt::vec3(0.0f, 1.0f, 0.0f));
    canvas_->setCamera(camera_);
	
	// init trackball and connect it to the canvas	
    tgt::Trackball* trackball_ = new tgt::Trackball(canvas_, true);
    trackball_->setCenter(tgt::vec3(0.f));	

    // init painter and connect it to the canvas
	VoreenPainter* painter_ = new VoreenPainter(canvas_, trackball_);
    canvas_->setPainter(painter_);
	// inform the message distributor about the new painter
    MsgDistr.insert(painter_);

    // init trackball navigation add a listener for that
	TrackballNavigation* trackNavi_ = new TrackballNavigation(trackball_, true, 0.05f, 15.f);
	canvas_->getEventHandler()->addListenerToBack(trackNavi_);
      

    // initialize the network serializer used for loading the network file
    NetworkSerializer* networkSerializer_ = new NetworkSerializer();
    // load processors from the network
    ProcessorNetwork net = networkSerializer_->readNetworkFromFile("../../data/networks/standard.vnw");

    std::vector<Processor*> processors = net.processors;
    // each processor should use the camera we created above
    for (size_t i = 0 ; i < processors.size() ; ++i)
        processors.at(i)->setCamera(camera_);

    // initialize the network evaluator, which -among others- tests the network for errors
    NetworkEvaluator* networkEvaluator_ = new NetworkEvaluator();
    // give the processors to the network evaluator
    networkEvaluator_->setProcessors(processors);

    // create a texture container with the final rendering target "20"
    networkEvaluator_->initTextureContainer(20);
    
    // inform the message distributor about the network evaluator
    MsgDistr.insert(networkEvaluator_);
    
    // tests if the network has any loops, all ports are connected and so on
    networkEvaluator_->analyze();

    // connect the painter with the evaluator
    painter_->setEvaluator(networkEvaluator_);


    // create a new transfer function object
    TransFuncIntensity* transferFunction_ = new TransFuncIntensity();
	// load the transfer function
    transferFunction_->load("../../data/transferfuncs/pet_sample.tfi");
    // build a texture out of the data contained in the transfer function
    transferFunction_->updateTexture();
    // every interested reciever should be informed that we have a new transfer function
    MsgDistr.postMessage(new TransFuncPtrMsg(VolumeRenderer::setTransFunc_, transferFunction_));


    // initialize a new volumeset container
    VolumeSetContainer* volumeSetContainer_ = new VolumeSetContainer();
    
    // initialize the volume serializer populator which will supply a volume serializer
    // that will be able to load a wide range of volumes
    VolumeSerializerPopulator* volumeSerializerPopulator_ = new VolumeSerializerPopulator();
    // get the volume serializer
    VolumeSerializer* volumeSerializer_ = volumeSerializerPopulator_->getVolumeSerializer();
    // load the volume...
    VolumeSet* volumeSet_ = volumeSerializer_->load("../../data/nucleon.dat");
    // ... and add it to the volume
    volumeSetContainer_->addVolumeSet(volumeSet_);


    // inform everybody about the new threshold we'd like to use
	MsgDistr.postMessage(new FloatMsg(VolumeRenderer::setLowerThreshold_, threshold_));
}

void keyPressed(unsigned char key, int /*x*/, int /*y*/) {
    switch (key) {
    case '\033': // = ESC
        exit(0);
        break;
    case '+':
        // increase threshold
        threshold_ = ((threshold_+0.02)<(1.0))?(threshold_+0.1):(1.0);
        // send the new threshold to the renderer
        MsgDistr.postMessage(new FloatMsg(VolumeRenderer::setLowerThreshold_, threshold_));
        // cause the renderer to rerender the scene
        MsgDistr.postMessage(new Message(VoreenPainter::repaint_) , VoreenPainter::visibleViews_);  
        break;
    case '-':
        // decreasing threshold
        threshold_ = ((threshold_ - 0.02) > 0.0) ? (threshold_ - 0.1) : 0.0;
        // send the new threshold to the renderer
        MsgDistr.postMessage(new FloatMsg(VolumeRenderer::setLowerThreshold_, threshold_));
        // cause the renderer to rerender the scene
        MsgDistr.postMessage(new Message(VoreenPainter::repaint_));   			
        break;
    case '1':
        glutReshapeWindow(128,128);
        break;
    case '2':
        glutReshapeWindow(256,256);
        break;
    case '3':
        glutReshapeWindow(512,512);
        break;
    case '4':
        glutReshapeWindow(1024, 1024);
        break;
    }
}

int main(int argc, char** argv) {
    tgt::init();
    glutInit(&argc, argv);	    

	// initialize canvas
	canvas_ = new tgt::GLUTCanvas("Voreen - The Volume Rendering Engine", tgt::ivec2(512, 512), tgt::GLCanvas::RGBADD); 
	canvas_->init();

	glutKeyboardFunc(keyPressed);

	init();

    glutMainLoop();
    
    tgt::deinit();
	return 0;
}
