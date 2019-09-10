/**
 * @file pynuitrack.cpp
 * @author Silas Alves (silas.alves)
 * @brief Contains the Nuitrack and NuitrackException claeses.
 * @version 0.1
 * @date 2019-09-03
 * 
 * @copyright Copyright (c) 2019
 * 
 * MIT License
 * 
 * Copyright (c) 2019 Silas Franco dos Reis Alves
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */

#include "pynuitrack.hpp"
#include <boost/algorithm/string.hpp>

namespace nt = tdv::nuitrack;
namespace bp = boost::python;
namespace np = boost::python::numpy;

NuitrackException::NuitrackException(std::string message)
{
    this->message = message;
}

const char *NuitrackException::what() const throw()
{
    return this->message.c_str();
}

NuitrackException::~NuitrackException() throw()
{
}

void translateException(NuitrackException const &e)
{
    PyErr_SetString(PyExc_RuntimeError, e.what());
}

Nuitrack::Nuitrack()
{
    _pyDepthCallback = NULL;
    _pyColorCallback = NULL;
    _pySkeletonCallback = NULL;
    _pyHandsCallback = NULL;
    _pyUserCallback = NULL;
    _pyGestureCallback = NULL;
    _pyIssueCallback = NULL;
    _pyFaceCallback = NULL;

    _yaml = bp::import("yaml");

    _collections = bp::import("collections");
    _namedtuple = _collections.attr("namedtuple");

    bp::list fieldsSkelResult;
    fieldsSkelResult.append("timestamp");
    fieldsSkelResult.append("skeleton_num");
    fieldsSkelResult.append("skeletons");
    _SkelResult = _namedtuple("SkeletonResult", fieldsSkelResult);

    bp::list fieldsSkeleton;
    fieldsSkeleton.append("userId");
    fieldsSkeleton.append("head");
    fieldsSkeleton.append("neck");
    fieldsSkeleton.append("torso");
    fieldsSkeleton.append("waist");
    fieldsSkeleton.append("left_collar");
    fieldsSkeleton.append("left_shoulder");
    fieldsSkeleton.append("left_elbow");
    fieldsSkeleton.append("left_wrist");
    fieldsSkeleton.append("left_hand");
    fieldsSkeleton.append("right_collar");
    fieldsSkeleton.append("right_shoulder");
    fieldsSkeleton.append("right_elbow");
    fieldsSkeleton.append("right_wrist");
    fieldsSkeleton.append("right_hand");
    fieldsSkeleton.append("left_hip");
    fieldsSkeleton.append("left_knee");
    fieldsSkeleton.append("left_ankle");
    fieldsSkeleton.append("right_hip");
    fieldsSkeleton.append("right_knee");
    fieldsSkeleton.append("right_ankle");
    _Skeleton = _namedtuple("Skeleton", fieldsSkeleton);

    bp::list fieldsJoint;
    fieldsJoint.append("type");
    fieldsJoint.append("confidence");
    fieldsJoint.append("real");
    fieldsJoint.append("projection");
    fieldsJoint.append("orientation");
    _Joint = _namedtuple("Joint", fieldsJoint);

    bp::list fieldsUserHands;
    fieldsUserHands.append("userId");
    fieldsUserHands.append("left");
    fieldsUserHands.append("right");
    _UserHands = _namedtuple("UserHands", fieldsUserHands);

    bp::list fieldsHand;
    fieldsHand.append("click");
    fieldsHand.append("pressure");
    fieldsHand.append("proj");
    fieldsHand.append("real");
    _Hand = _namedtuple("Hand", fieldsHand);

    bp::list fieldsGesture;
    fieldsGesture.append("userId");
    fieldsGesture.append("type");
    _Gesture = _namedtuple("Gesture", fieldsGesture);

    bp::list fieldsFBIssue;
    fieldsFBIssue.append("userId");
    fieldsFBIssue.append("left");
    fieldsFBIssue.append("right");
    fieldsFBIssue.append("top");
    _FrameBorderIssue = _namedtuple("FrameBorderIssue", fieldsFBIssue);

    bp::list fieldsOIssue;
    fieldsOIssue.append("userId");
    _OcclusionIssue = _namedtuple("OcclusionIssue", fieldsOIssue);
}

void Nuitrack::init(std::string configPath)
{
    // Initialize Nuitrack
    try
    {
        nt::Nuitrack::init(configPath);
    }
    catch (const nt::Exception &e)
    {
        throw NuitrackException("Could not initialize Nuitrack");
    }

    // These two settings are required to enable face tracking.
    nt::Nuitrack::setConfigValue("Faces.ToUse", "true");
    nt::Nuitrack::setConfigValue("DepthProvider.Depth2ColorRegistration", "true");

    _depthSensor = nt::DepthSensor::create();
    _depthSensor->connectOnNewFrame(
        std::bind(&Nuitrack::_onNewDepthFrame, this, std::placeholders::_1));
    _outputModeDepth = _depthSensor->getOutputMode();

    _colorSensor = nt::ColorSensor::create();
    _colorSensor->connectOnNewFrame(
        std::bind(&Nuitrack::_onNewRGBFrame, this, std::placeholders::_1));
    _outputModeColor = _colorSensor->getOutputMode();

    _handTracker = nt::HandTracker::create();
    _handTracker->connectOnUpdate(
        std::bind(&Nuitrack::_onHandUpdate, this, std::placeholders::_1));

    _userTracker = nt::UserTracker::create();
    _userTracker->connectOnUpdate(
        std::bind(&Nuitrack::_onUserUpdate, this, std::placeholders::_1));

    _skeletonTracker = nt::SkeletonTracker::create();
    _skeletonTracker->connectOnUpdate(
        std::bind(&Nuitrack::_onSkeletonUpdate, this, std::placeholders::_1));

    _gestureRecognizer = nt::GestureRecognizer::create();
    _gestureRecognizer->connectOnNewGestures(
        std::bind(&Nuitrack::_onNewGesture, this, std::placeholders::_1));

    _onIssuesUpdateHandler = nt::Nuitrack::connectOnIssuesUpdate(
        std::bind(&Nuitrack::_onIssuesUpdate, this, std::placeholders::_1));

    // Start Nuitrack
    try
    {
        nt::Nuitrack::run();
    }
    catch (const nt::Exception &e)
    {
        std::string msg("Nuitrack update failed: ");
        msg += exceptionType_str[e.type()];
        throw NuitrackException(msg);
    }
}

void Nuitrack::update()
{
    try
    {
        nt::Nuitrack::waitUpdate(_skeletonTracker);
    }
    catch (nt::LicenseNotAcquiredException &e)
    {
        throw NuitrackException("License not acquired.");
    }
    catch (const nt::Exception &e)
    {
        std::string msg("Nuitrack update failed: ");
        msg += exceptionType_str[e.type()];
        throw NuitrackException(msg);
    }
}

void Nuitrack::setDepthCallback(PyObject *callable)
{
    _pyDepthCallback = callable;
}

void Nuitrack::setColorCallback(PyObject *callable)
{
    _pyColorCallback = callable;
}

void Nuitrack::setSkeletonCallback(PyObject *callable)
{
    _pySkeletonCallback = callable;
}

void Nuitrack::setFaceCallback(PyObject *callable)
{
    _pyFaceCallback = callable;
}

void Nuitrack::setHandsCallback(PyObject *callable)
{
    _pyHandsCallback = callable;
}

void Nuitrack::Nuitrack::setUserCallback(PyObject *callable)
{
    _pyUserCallback = callable;
}

void Nuitrack::setGestureCallback(PyObject *callable)
{
    _pyGestureCallback = callable;
}

void Nuitrack::setIssueCallback(PyObject *callable)
{
    _pyIssueCallback = callable;
}

void Nuitrack::_onIssuesUpdate(nt::IssuesData::Ptr issuesData)
{
    if (_pyIssueCallback && issuesData)
    {
        bp::list listIssues;
        for (int userId = 0; userId < 8; userId++)
        {
            auto issueFB = issuesData->getUserIssue<nt::FrameBorderIssue>(userId);
            if (issueFB)
                listIssues.append(_FrameBorderIssue(userId, 
                                                    issueFB->isLeft(),
                                                    issueFB->isRight(),
                                                    issueFB->isTop()));

            auto issueOcc = issuesData->getUserIssue<nt::OcclusionIssue>(userId);
            if (issueOcc)
                listIssues.append(_OcclusionIssue(userId));
        }

        if (bp::len(listIssues))
            bp::call<void>(_pyIssueCallback, listIssues);
    }
}

void Nuitrack::_onNewGesture(nt::GestureData::Ptr gestureData)
{
    if (_pyGestureCallback)
    {
        auto gestures = gestureData->getGestures();
        bp::list listGest;
        for (nt::Gesture gest : gestures)
        {
            listGest.append(_Gesture(gest.userId, gest.type));
        }
        bp::call<void>(_pyGestureCallback, listGest);
    }
}

void Nuitrack::_onUserUpdate(nt::UserFrame::Ptr frame)
{
    if (_pyUserCallback != NULL)
    {
        const uint16_t *depthPtr = frame->getData();
        int nCols = frame->getCols();
        int nRows = frame->getRows();

        np::ndarray npData = np::from_data(
                    depthPtr, _dtUInt16,
                    bp::make_tuple(nRows, nCols),
                    bp::make_tuple(nCols * sizeof(uint16_t), sizeof(uint16_t)),
                    bp::object());

        bp::call<void>(_pyUserCallback, npData.copy());
    }
}

bp::api::object Nuitrack::_getJointData(nt::Joint joint)
{
    float fReal[] = {joint.real.x, joint.real.y, joint.real.z};

    np::ndarray real = np::from_data(fReal, _dtFloat,
                                     bp::make_tuple(3),
                                     bp::make_tuple(sizeof(float)),
                                     bp::object());

    float fProj[] = {joint.proj.x * _outputModeColor.xres,
                     joint.proj.y * _outputModeColor.yres,
                     joint.proj.z};

    np::ndarray proj = np::from_data(fProj, _dtFloat,
                                     bp::make_tuple(3),
                                     bp::make_tuple(sizeof(float)),
                                     bp::object());

    np::ndarray orientation = np::from_data(
                joint.orient.matrix, _dtFloat,
                bp::make_tuple(3, 3),
                bp::make_tuple(3 * sizeof(float), sizeof(float)),
                bp::object());

    return _Joint(joint.type,
                  joint.confidence,
                  real.copy(),
                  proj.copy(),
                  orientation.copy());
}

void Nuitrack::_onSkeletonUpdate(nt::SkeletonData::Ptr userSkeletons)
{
    if (_pySkeletonCallback)
    {
        bp::list listSkel;
        auto skeletons = userSkeletons->getSkeletons();
        for (nt::Skeleton skel : skeletons)
        {
            bp::list listJoint;
            listJoint.append(skel.id);
            listJoint.append(_getJointData(skel.joints[nt::JOINT_HEAD]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_NECK]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_TORSO]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_WAIST]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_LEFT_COLLAR]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_LEFT_SHOULDER]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_LEFT_ELBOW]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_LEFT_WRIST]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_LEFT_HAND]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_RIGHT_COLLAR]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_RIGHT_SHOULDER]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_RIGHT_ELBOW]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_RIGHT_WRIST]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_RIGHT_HAND]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_LEFT_HIP]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_LEFT_KNEE]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_LEFT_ANKLE]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_RIGHT_HIP]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_RIGHT_KNEE]));
            listJoint.append(_getJointData(skel.joints[nt::JOINT_RIGHT_ANKLE]));
            listSkel.append(_Skeleton.attr("_make")(listJoint));
        }

        auto data = _SkelResult(
            userSkeletons->getTimestamp(),
            userSkeletons->getNumSkeletons(),
            listSkel);

        bp::call<void>(_pySkeletonCallback, data);
    }

    if (_pyFaceCallback)
    {
        std::string faceInfo = nt::Nuitrack::getInstancesJson();

        // Remove the quotes from the JSON file so the (un)quoted numbers can
        // be read as int or float. Uses PyYAML for parsing.
        boost::replace_all(faceInfo, "\"", "");
        bp::call<void>(_pyFaceCallback, _yaml.attr("load")(faceInfo));
    }
}

void Nuitrack::_onNewDepthFrame(nt::DepthFrame::Ptr frame)
{
    if (_pyDepthCallback != NULL)
    {
        const uint16_t *depthPtr = frame->getData();
        int nCols = frame->getCols();
        int nRows = frame->getRows();

        np::ndarray npData = np::from_data(
                    depthPtr, _dtUInt16,
                    bp::make_tuple(nRows, nCols),
                    bp::make_tuple(nCols * sizeof(uint16_t), sizeof(uint16_t)),
                    bp::object());

        bp::call<void>(_pyDepthCallback, npData.copy());
    }
}

void Nuitrack::_onNewRGBFrame(nt::RGBFrame::Ptr frame)
{
    if (_pyColorCallback != NULL)
    {
        const uint8_t *colorPtr = (uint8_t *)frame->getData();
        int nCols = frame->getCols();
        int nRows = frame->getRows();

        np::ndarray npData = np::from_data(
            colorPtr, _dtUInt8,
            bp::make_tuple(nRows, nCols, 3),
            bp::make_tuple(nCols * 3 * sizeof(uint8_t), 3 * sizeof(uint8_t), sizeof(uint8_t)),
            bp::object());

        bp::call<void>(_pyColorCallback, npData.copy());
    }
}

bp::api::object Nuitrack::_getHandData(nt::Hand::Ptr hand)
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
void Nuitrack::_onHandUpdate(nt::HandTrackerData::Ptr handData)
{
    if (_pyHandsCallback && handData)
    {
        bp::list listUserHands;

        for (nt::UserHands hands : handData->getUsersHands())
        {
            auto data = _UserHands(
                hands.userId,
                _getHandData(hands.leftHand),
                _getHandData(hands.leftHand));

            listUserHands.append(data);
        }

        auto data = bp::make_tuple(
            handData->getTimestamp(),
            handData->getNumUsers(),
            listUserHands);

        bp::call<void>(_pyHandsCallback, data);
    }
}

void Nuitrack::release()
{
    nt::Nuitrack::release();
}

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(nt_init_overloads, Nuitrack::init, 0, 1)

BOOST_PYTHON_MODULE(pynuitrack)
{
    Py_Initialize();
    np::initialize();

    bp::register_exception_translator<NuitrackException>(&translateException);
    bp::register_exception_translator<NuitrackInitFail>(&translateException);

    bp::enum_<nt::GestureType>("GestureType")
        .value("waving", nt::GESTURE_WAVING)
        .value("swipe_left", nt::GESTURE_SWIPE_LEFT)
        .value("swipe_right", nt::GESTURE_SWIPE_RIGHT)
        .value("swipe_up", nt::GESTURE_SWIPE_UP)
        .value("swipe_down", nt::GESTURE_SWIPE_DOWN)
        .value("push", nt::GESTURE_PUSH)
        .export_values();

    bp::enum_<nt::JointType>("JointType")
        .value("none", nt::JOINT_NONE)
        .value("head", nt::JOINT_HEAD)
        .value("neck", nt::JOINT_NECK)
        .value("torso", nt::JOINT_TORSO)
        .value("waist", nt::JOINT_WAIST)
        .value("left_collar", nt::JOINT_LEFT_COLLAR)
        .value("left_shoulder", nt::JOINT_LEFT_SHOULDER)
        .value("left_elbow", nt::JOINT_LEFT_ELBOW)
        .value("left_wrist", nt::JOINT_LEFT_WRIST)
        .value("left_hand", nt::JOINT_LEFT_HAND)
        .value("left_fingertip", nt::JOINT_LEFT_FINGERTIP)
        .value("right_collar", nt::JOINT_RIGHT_COLLAR)
        .value("right_shoulder", nt::JOINT_RIGHT_SHOULDER)
        .value("right_elbow", nt::JOINT_RIGHT_ELBOW)
        .value("right_wrist", nt::JOINT_RIGHT_WRIST)
        .value("right_hand", nt::JOINT_RIGHT_HAND)
        .value("right_fingertip", nt::JOINT_RIGHT_FINGERTIP)
        .value("left_hip", nt::JOINT_LEFT_HIP)
        .value("left_knee", nt::JOINT_LEFT_KNEE)
        .value("left_ankle", nt::JOINT_LEFT_ANKLE)
        .value("left_foot", nt::JOINT_LEFT_FOOT)
        .value("right_hip", nt::JOINT_RIGHT_HIP)
        .value("right_knee", nt::JOINT_RIGHT_KNEE)
        .value("right_ankle", nt::JOINT_RIGHT_ANKLE)
        .value("right_foot", nt::JOINT_RIGHT_FOOT)
        .export_values();

    bp::class_<Nuitrack>("Nuitrack", bp::init<>())
        .def("init", &Nuitrack::init, nt_init_overloads(bp::arg("configPath") = "", "Path to the configuration file"))
        .def("release", &Nuitrack::release)
        .def("set_depth_callback", &Nuitrack::setDepthCallback)
        .def("set_color_callback", &Nuitrack::setColorCallback)
        .def("set_skeleton_callback", &Nuitrack::setSkeletonCallback)
        .def("set_face_callback", &Nuitrack::setFaceCallback)
        .def("set_hands_callback", &Nuitrack::setHandsCallback)
        .def("set_user_callback", &Nuitrack::setUserCallback)
        .def("set_gesture_callback", &Nuitrack::setGestureCallback)
        .def("set_issue_callback", &Nuitrack::setIssueCallback)
        .def("update", &Nuitrack::update);
};
