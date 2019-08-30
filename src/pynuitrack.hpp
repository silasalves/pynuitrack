#ifndef pynuitrack_H
#define pynuitrack_H

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <nuitrack/Nuitrack.h>

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
	uint64_t _onIssuesUpdateHandler;
    
  PyObject* _pyDepthCallback;
  PyObject* _pyColorCallback;
  PyObject* _pySkeletonCallback;
  PyObject* _pyHandsCallback;
  PyObject* _pyUserCallback;
  PyObject* _pyGestureCallback;
  PyObject* _pyIssueCallback;

  boost::python::numpy::dtype _dtUInt8 = boost::python::numpy::dtype::get_builtin<uint8_t>();
  boost::python::numpy::dtype _dtUInt16 = boost::python::numpy::dtype::get_builtin<uint16_t>();
  boost::python::numpy::dtype _dtFloat = boost::python::numpy::dtype::get_builtin<float>();

  boost::python::api::object _collections;
  boost::python::api::object _namedtuple;
  boost::python::api::object _Joint;
  boost::python::api::object _Hand;
  boost::python::api::object _UserHands;
  boost::python::api::object _Gesture;
  boost::python::api::object _FrameBorderIssue;
  boost::python::api::object _OcclusionIssue;

public:
    Nuitrack();

    void init(std::string configPath = "");

    void update();

    void setDepthCallback(PyObject* callable);

    void setColorCallback(PyObject* callable);

    void setSkeletonCallback(PyObject* callable);

    void setHandsCallback(PyObject* callable);

    void setUserCallback(PyObject* callable);

    void setGestureCallback(PyObject* callable);

    void setIssueCallback(PyObject* callable);

    void onIssuesUpdate(tdv::nuitrack::IssuesData::Ptr issuesData);

    void onNewGesture(tdv::nuitrack::GestureData::Ptr gestureData);

    void onUserUpdate(tdv::nuitrack::UserFrame::Ptr frame);

    boost::python::api::object _extractJointData(tdv::nuitrack::Joint joint);

    void onSkeletonUpdate(tdv::nuitrack::SkeletonData::Ptr userSkeletons);

    void onNewDepthFrame(tdv::nuitrack::DepthFrame::Ptr frame);

    void onNewRGBFrame(tdv::nuitrack::RGBFrame::Ptr frame);

    boost::python::api::object _extractHandData(tdv::nuitrack::Hand::Ptr hand);

    void onHandUpdate(tdv::nuitrack::HandTrackerData::Ptr handData);

    void release();
};

class NuitrackException : public std::exception
{
private:
    std::string message;

public:
    NuitrackException(std::string message);
    const char *what() const throw();
    ~NuitrackException() throw();
};

class NuitrackInitFail : public NuitrackException {};

void translateException(NuitrackException const& e);

const char *exceptionType_str[] = {
  "OK",
  "Exception",
  "Terminated",
  "Bad configuration value",
  "Configuration not found",
  "Module not found",
  "License not acquired",
  "Module not initialized",
  "Module not started"
};

#endif