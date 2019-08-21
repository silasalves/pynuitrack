#include "pynuitrack.hpp"

#include <boost/python.hpp>
#include <boost/python/list.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/exception_translator.hpp>
#include <boost/python/numpy.hpp>

#include <string>
#include <sstream>
#include <vector>

#include <nuitrack/Nuitrack.h>
#include <iomanip>

namespace nt = tdv::nuitrack;
namespace bp = boost::python;
namespace np = boost::python::numpy;


class NuitrackException : public std::exception
{
private:
    std::string message;

public:
    NuitrackException(std::string message)
    {
        this->message = message;
    }

    const char *what() const throw()
    {
        return this->message.c_str();
    }

    ~NuitrackException() throw()
    {
    }
};

class NuitrackInitFail : public NuitrackException {};

void translateException(NuitrackException const& e)
{
    PyErr_SetString(PyExc_RuntimeError, e.what());
}

class Nuitrack
{
private:
    tdv::nuitrack::OutputMode _outputModeDepth;
    tdv::nuitrack::OutputMode _outputModeColor;
	tdv::nuitrack::DepthSensor::Ptr _depthSensor;
	tdv::nuitrack::ColorSensor::Ptr _colorSensor;
	tdv::nuitrack::UserTracker::Ptr _userTracker;
	tdv::nuitrack::SkeletonTracker::Ptr _skeletonTracker;
	tdv::nuitrack::HandTracker::Ptr _handTracker;
	tdv::nuitrack::GestureRecognizer::Ptr _gestureRecognizer;
	tdv::nuitrack::IssuesData::Ptr _issuesData;

    PyObject* _pyDepthCallback;
    PyObject* _pyColorCallback;
    PyObject* _pySkeletonCallback;
    PyObject* _pyHandsCallback;
    PyObject* _pyUserCallback;

    np::dtype _dtUInt8 = np::dtype::get_builtin<uint8_t>();
    np::dtype _dtUInt16 = np::dtype::get_builtin<uint16_t>();
    np::dtype _dtFloat = np::dtype::get_builtin<float>();

    bp::api::object _collections;
    bp::api::object _namedtuple;
    bp::api::object _Joint;
    bp::api::object _Hand;
    bp::api::object _UserHands;

public:
    Nuitrack()
    {
        _pyDepthCallback = NULL;
        _pyColorCallback = NULL;
        _pySkeletonCallback = NULL;
        _pyHandsCallback = NULL;
        _pyUserCallback = NULL;

        _collections = bp::import("collections");
        _namedtuple = _collections.attr("namedtuple");

        bp::list fieldsJoint;
        fieldsJoint.append("type");
        fieldsJoint.append("confidence");
        fieldsJoint.append("real");
        fieldsJoint.append("projection");
        fieldsJoint.append("orientation");
        _Joint = _namedtuple("Joint", fieldsJoint);

        bp::list fieldsUserHands;
        fieldsUserHands.append("userID");
        fieldsUserHands.append("left");
        fieldsUserHands.append("right");
        _UserHands = _namedtuple("UserHands", fieldsUserHands);

        bp::list fieldsHand;
        fieldsHand.append("click");
        fieldsHand.append("pressure");
        fieldsHand.append("proj");
        fieldsHand.append("real");
        _Hand = _namedtuple("Hand", fieldsHand);
    }

    void init(std::string configPath = "")
    {
        // Initialize Nuitrack
        try
        {
            tdv::nuitrack::Nuitrack::init(configPath);
        }
        catch (const tdv::nuitrack::Exception& e)
        {
            throw NuitrackException("Could not initialize Nuitrack");
        }

        _depthSensor = tdv::nuitrack::DepthSensor::create();
        _depthSensor->connectOnNewFrame(std::bind(&Nuitrack::onNewDepthFrame, this, std::placeholders::_1));
        _outputModeDepth = _depthSensor->getOutputMode();

        _colorSensor = tdv::nuitrack::ColorSensor::create();
        _colorSensor->connectOnNewFrame(std::bind(&Nuitrack::onNewRGBFrame, this, std::placeholders::_1));
        _outputModeColor = _colorSensor->getOutputMode();

        _handTracker = tdv::nuitrack::HandTracker::create();
        _handTracker->connectOnUpdate(std::bind(&Nuitrack::onHandUpdate, this, std::placeholders::_1));

        // _userTracker = UserTracker::create();
        // // Bind to event update user tracker
        // _userTracker->connectOnUpdate(std::bind(&NuitrackGLSample::onUserUpdate, this, std::placeholders::_1));

        _skeletonTracker = tdv::nuitrack::SkeletonTracker::create();
        _skeletonTracker->connectOnUpdate(std::bind(&Nuitrack::onSkeletonUpdate, this, std::placeholders::_1));

        // _gestureRecognizer = GestureRecognizer::create();
        // _gestureRecognizer->connectOnNewGestures(std::bind(&NuitrackGLSample::onNewGesture, this, std::placeholders::_1));

        // _onIssuesUpdateHandler = Nuitrack::connectOnIssuesUpdate(std::bind(&NuitrackGLSample::onIssuesUpdate,
        //                                                                   this, std::placeholders::_1));


        // Start Nuitrack
        try
        {
            tdv::nuitrack::Nuitrack::run();
        }
        catch (const tdv::nuitrack::Exception& e)
        {
            std::string msg("Nuitrack update failed: ");
            msg += exceptionType_str[e.type()];
            throw NuitrackException(msg);
        }
    }

    void update()
    {
        try
        {
            tdv::nuitrack::Nuitrack::waitUpdate(_skeletonTracker);
        }
        catch (tdv::nuitrack::LicenseNotAcquiredException& e)
        {
            throw NuitrackException("License not acquired.");
        }
        catch (const tdv::nuitrack::Exception& e)
        {
            std::string msg("Nuitrack update failed: ");
            msg += exceptionType_str[e.type()];
            throw NuitrackException(msg);
        }
    }

    void setDepthCallback(PyObject* callable)
    {
        _pyDepthCallback = callable;
    }

    void setColorCallback(PyObject* callable)
    {
        _pyColorCallback = callable;
    }

    void setSkeletonCallback(PyObject* callable)
    {
        _pySkeletonCallback = callable;
    }

    void setHandsCallback(PyObject* callable)
    {
        _pyHandsCallback = callable;
    }

    bp::api::object _extractJointData(tdv::nuitrack::Joint joint)
    {
        float fReal[] = {joint.real.x, joint.real.y, joint.real.z};
        
        np::ndarray real = np::from_data(fReal, _dtFloat,
                                        bp::make_tuple(3),
                                        bp::make_tuple(sizeof(float)),
                                        bp::object());

        float fProj[] = {joint.proj.x * _outputModeColor.xres,
                         joint.proj.y * _outputModeColor.yres,
                         joint.proj.z };
        
        np::ndarray proj = np::from_data(fProj, _dtFloat,
                                        bp::make_tuple(3),
                                        bp::make_tuple(sizeof(float)),
                                        bp::object());
        
        np::ndarray orientation = np::from_data(joint.orient.matrix, _dtFloat,
                                        bp::make_tuple(3, 3),
                                        bp::make_tuple(3 * sizeof(float), sizeof(float)),
                                        bp::object());

        return _Joint((int)joint.type,
                      joint.confidence, 
                      real.copy(),
                      proj.copy(),
                      orientation.copy());
    }

    void onSkeletonUpdate(tdv::nuitrack::SkeletonData::Ptr userSkeletons)
    {
        if (_pySkeletonCallback)
        {
            bp::list listSkel;
            auto skeletons = userSkeletons->getSkeletons();
            for (tdv::nuitrack::Skeleton skeleton: skeletons)
            {
                bp::list listJoint;
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_HEAD]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_NECK]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_TORSO]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_WAIST]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_LEFT_COLLAR]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_LEFT_SHOULDER]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_LEFT_ELBOW]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_LEFT_WRIST]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_LEFT_HAND]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_RIGHT_COLLAR]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_RIGHT_SHOULDER]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_RIGHT_ELBOW]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_RIGHT_WRIST]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_RIGHT_HAND]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_LEFT_HIP]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_LEFT_KNEE]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_LEFT_ANKLE]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_RIGHT_HIP]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_RIGHT_KNEE]));
                listJoint.append(_extractJointData(skeleton.joints[nt::JOINT_RIGHT_ANKLE]));
                listSkel.append(listJoint);
            }

            bp::tuple data =  bp::make_tuple(
                userSkeletons->getTimestamp(),
                userSkeletons->getNumSkeletons(),
                listSkel);
            
            boost::python::call<void>(_pySkeletonCallback, data);
        }
    }


    void onNewDepthFrame(tdv::nuitrack::DepthFrame::Ptr frame)
    {
        if (_pyDepthCallback != NULL)
        {
            const uint16_t* depthPtr = frame->getData();
            int nCols = frame->getCols();
	        int nRows = frame->getRows();

            np::ndarray npData = np::from_data(depthPtr, _dtUInt16,
                                            bp::make_tuple(nRows, nCols),
                                            bp::make_tuple(nCols * sizeof(uint16_t), sizeof(uint16_t)),
                                            bp::object());

            boost::python::call<void>(_pyDepthCallback, npData.copy());
        }
    }

    void onNewRGBFrame(tdv::nuitrack::RGBFrame::Ptr frame)
    {
        if (_pyColorCallback != NULL)
        {
            const uint8_t* colorPtr = (uint8_t*) frame->getData();
            int nCols = frame->getCols();
	        int nRows = frame->getRows();

            np::ndarray npData = np::from_data(colorPtr, _dtUInt8,
                                            bp::make_tuple(nRows, nCols, 3),
                                            bp::make_tuple(nCols * 3 * sizeof(uint8_t), 3 * sizeof(uint8_t), sizeof(uint8_t)),
                                            bp::object());

            boost::python::call<void>(_pyColorCallback, npData.copy());
        }
    }

    bp::api::object _extractHandData(nt::Hand::Ptr hand)
    {
        if (hand && hand->x != -1)
        {
            float fProj[] = {hand->x * _outputModeColor.xres,
                             hand->y * _outputModeColor.yres};
    
            np::ndarray proj = np::from_data(fProj, _dtFloat,
                                            bp::make_tuple(2),
                                            bp::make_tuple(sizeof(float)),
                                            bp::object());
            
            float fReal[] = {hand->xReal, hand->yReal, hand->zReal};
    
            np::ndarray real = np::from_data(fReal, _dtFloat,
                                            bp::make_tuple(3),
                                            bp::make_tuple(sizeof(float)),
                                            bp::object());
            
            return _Hand(hand->click, hand->pressure, proj.copy(), real.copy());
        }
        else
            return bp::object();
    }

    // Callback for the hand data update event
    void onHandUpdate(nt::HandTrackerData::Ptr handData)
    {
        if (_pyHandsCallback && handData)
        {
            bp::list listUserHands;

            for (nt::UserHands hands : handData->getUsersHands())
            {
                auto data = _UserHands(
                    hands.userId,
                    _extractHandData(hands.leftHand), 
                    _extractHandData(hands.leftHand));

                listUserHands.append(data);
            }

            auto data = bp::make_tuple(
                handData->getTimestamp(),
                handData->getNumUsers(),
                listUserHands
            );

            bp::call<void>(_pyHandsCallback, data);
        }
    }

    void release()
    {
        tdv::nuitrack::Nuitrack::release();
    }
};

using namespace boost::python;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(nt_init_overloads, Nuitrack::init, 0, 1)

BOOST_PYTHON_MODULE(pynuitrack)
{
    Py_Initialize();
    np::initialize();

    register_exception_translator<NuitrackException>(&translateException);
    register_exception_translator<NuitrackInitFail>(&translateException);

    class_<Nuitrack>("Nuitrack", boost::python::init<>())
        .def("init", &Nuitrack::init, nt_init_overloads(
            boost::python::arg("configPath")="", 
            "Path to the configuration file"
        ))
        .def("release", &Nuitrack::release)
        .def("set_depth_callback", &Nuitrack::setDepthCallback)
        .def("set_color_callback", &Nuitrack::setColorCallback)
        .def("set_skeleton_callback", &Nuitrack::setSkeletonCallback)
        .def("set_hands_callback", &Nuitrack::setHandsCallback)
        .def("update", &Nuitrack::update)
    ;
};
