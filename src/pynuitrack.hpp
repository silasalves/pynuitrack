#ifndef pynuitrack_H
#define pynuitrack_H

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <nuitrack/Nuitrack.h>

namespace nt = tdv::nuitrack;
namespace bp = boost::python;
namespace np = boost::python::numpy;

class Nuitrack
{
private:
  nt::OutputMode _outputModeDepth;
  nt::OutputMode _outputModeColor;
	nt::DepthSensor::Ptr _depthSensor;
	nt::ColorSensor::Ptr _colorSensor;
	nt::UserTracker::Ptr _userTracker;
	nt::SkeletonTracker::Ptr _skeletonTracker;
	nt::HandTracker::Ptr _handTracker;
	nt::GestureRecognizer::Ptr _gestureRecognizer;
	uint64_t _onIssuesUpdateHandler;
    
  PyObject* _pyDepthCallback;
  PyObject* _pyColorCallback;
  PyObject* _pySkeletonCallback;
  PyObject* _pyHandsCallback;
  PyObject* _pyUserCallback;
  PyObject* _pyGestureCallback;
  PyObject* _pyIssueCallback;

  np::dtype _dtUInt8 = np::dtype::get_builtin<uint8_t>();
  np::dtype _dtUInt16 = np::dtype::get_builtin<uint16_t>();
  np::dtype _dtFloat = np::dtype::get_builtin<float>();

  bp::api::object _collections;
  bp::api::object _namedtuple;
  bp::api::object _Joint;
  bp::api::object _Hand;
  bp::api::object _UserHands;
  bp::api::object _Gesture;
  bp::api::object _FrameBorderIssue;
  bp::api::object _OcclusionIssue;

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

    void onIssuesUpdate(nt::IssuesData::Ptr issuesData);

    void onNewGesture(nt::GestureData::Ptr gestureData);

    void onUserUpdate(nt::UserFrame::Ptr frame);

    bp::api::object _extractJointData(nt::Joint joint);

    void onSkeletonUpdate(nt::SkeletonData::Ptr userSkeletons);

    void onNewDepthFrame(nt::DepthFrame::Ptr frame);

    void onNewRGBFrame(nt::RGBFrame::Ptr frame);

    bp::api::object _extractHandData(nt::Hand::Ptr hand);

    void onHandUpdate(nt::HandTrackerData::Ptr handData);

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