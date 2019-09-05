/**
 * @file pynuitrack.hpp
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

#ifndef pynuitrack_H
#define pynuitrack_H

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <nuitrack/Nuitrack.h>

/**
 * @brief Provides access to the Nuitrack library.
 * 
 * This is the main class for pynuitrack. It manages the callbacks for the depth
 * image, color image (BGR), user tracker, skeleton tracker, hand tracker and
 * gesture recognizer.
 */
class Nuitrack
{
private:
    /// Stores the output mode for the depth image.
    tdv::nuitrack::OutputMode _outputModeDepth;

    /// Stores the output mode for the color image.
    tdv::nuitrack::OutputMode _outputModeColor;

    /// Handler for the depth image interface.
    tdv::nuitrack::DepthSensor::Ptr _depthSensor;

    /// Handler for the color image interface.
    tdv::nuitrack::ColorSensor::Ptr _colorSensor;

    /// Handler for the user tracker interface.
    tdv::nuitrack::UserTracker::Ptr _userTracker;

    /// Handler for the skeleton tracker interface.
    tdv::nuitrack::SkeletonTracker::Ptr _skeletonTracker;
    
    /// Handler for the hand tracker interface.
    tdv::nuitrack::HandTracker::Ptr _handTracker;

    /// Handler for the gesture recognizer interface.
    tdv::nuitrack::GestureRecognizer::Ptr _gestureRecognizer;

    /// Handler for the user tracker interface.
    uint64_t _onIssuesUpdateHandler;

    /// Python callback for the depth image.
    PyObject *_pyDepthCallback;

    /// Python callback for the color image.
    PyObject *_pyColorCallback;

    /// Python callback for the skeleton tracking.
    PyObject *_pySkeletonCallback;

    /// Python callback for the skeleton tracking.
    PyObject *_pyFaceCallback;

    /// Python callback for the hand tracking.
    PyObject *_pyHandsCallback;

    /// Python callback for the user tracking.
    PyObject *_pyUserCallback;

    /// Python callback for the gesture tracking.
    PyObject *_pyGestureCallback;

    /// Python callback for the issue handler.
    PyObject *_pyIssueCallback;

    /// Numpy representation of the uint8_t type.
    boost::python::numpy::dtype _dtUInt8 =
        boost::python::numpy::dtype::get_builtin<uint8_t>();

    /// Numpy representation of the uint16_t type.
    boost::python::numpy::dtype _dtUInt16 =
        boost::python::numpy::dtype::get_builtin<uint16_t>();

    /// Numpy representation of the float type.
    boost::python::numpy::dtype _dtFloat =
        boost::python::numpy::dtype::get_builtin<float>();
    
    /// Handler for Python JSON library. 
    boost::python::api::object _yaml;

    /// Handler for Python's collections package.
    boost::python::api::object _collections;

    /// Handler for Python's named tuple.
    boost::python::api::object _namedtuple;

    /// Named tuple "Joint", used by skeleton tracking.
    boost::python::api::object _Joint;

    /// Named tuple "Skeleton", used by skeleton tracking.
    boost::python::api::object _Skeleton;

    /// Named tuple "SkeletonResult", used by skeleton tracking.
    boost::python::api::object _SkelResult;

    /// Named tuple "Hand", used by hand tracking.
    boost::python::api::object _Hand;

    /// Named tuple "UserHand", used by hand tracking.
    boost::python::api::object _UserHands;

    /// Named tuple "Gesture", used by gesture tracking.
    boost::python::api::object _Gesture;

    /// Named tuple "FrameBorderIssue", used by issue tracking.
    boost::python::api::object _FrameBorderIssue;

    /// Named tuple "FrameBorderIssue", used by occlusion tracking.
    boost::python::api::object _OcclusionIssue;

    /**
     * @brief Callback method for the issue tracker.
     * 
     * @param issuesData 
     */
    void _onIssuesUpdate(tdv::nuitrack::IssuesData::Ptr issuesData);

    /**
     * @brief Callback method for the gesture recognizer.
     * 
     * @param gestureData 
     */
    void _onNewGesture(tdv::nuitrack::GestureData::Ptr gestureData);

    /**
     * @brief Callback method for the user tracker.
     * 
     * @param frame 
     */
    void _onUserUpdate(tdv::nuitrack::UserFrame::Ptr frame);

    /**
     * @brief Callback method for the skeleton tracker.
     * 
     * @param userSkeletons 
     */
    void _onSkeletonUpdate(tdv::nuitrack::SkeletonData::Ptr userSkeletons);

    /**
     * @brief Callback method for the depth sensor.
     * 
     * @param frame 
     */
    void _onNewDepthFrame(tdv::nuitrack::DepthFrame::Ptr frame);

    /**
     * @brief Callback method for the color camera.
     * 
     * @param frame 
     */
    void _onNewRGBFrame(tdv::nuitrack::RGBFrame::Ptr frame);

    /**
     * @brief Callback method for the hand tracker.
     * 
     * @param handData 
     */
    void _onHandUpdate(tdv::nuitrack::HandTrackerData::Ptr handData);

    /**
     * @brief Converts a Nuitrack Hand type to a python named tuple.
     * 
     * @param hand Hand object with the information of the given hand.
     * @return boost::python::api::object A named tuple with the same
     *      information as the input parameter.
     */
    boost::python::api::object _getHandData(tdv::nuitrack::Hand::Ptr hand);

    /**
     * @brief Converts a Nuitrack joint to a python named tuple.
     * 
     * @param joint Joint object with the information of a given skeleton joint.
     * @return boost::python::api::object A named tuple with the same
     *      information as the input parameter.
     */
    boost::python::api::object _getJointData(tdv::nuitrack::Joint joint);

public:
    /**
     * @brief Construct a new Nuitrack object.
     * 
     * Sets the python callbacks to Null and initializes the Python named
     * tuples.
     */
    Nuitrack();

    /**
     * @brief Python constructor for a new Nuitrack object.
     * 
     * This should be called before using any of the other methods.
     * 
     * @param configPath Path to Nuitrack configuration file.
     */
    void init(std::string configPath = "");

    /**
     * @brief Updates data from all Nuitrack modules and feed them to callbacks.
     * 
     * Requests new data from depth sensor, color camera, skeleton tracking,
     * hand tracking, user tracking, gesture tracking and issue monitoring.
     * After getting the new data, all callbacks will be called.
     */
    void update();

    /**
     * @brief Stops data processing and destroy all Nuitrack modules.
     */
    void release();

    /**
     * @brief Set the Python depth sensor callback.
     * 
     * @param callable A Python function.
     */
    void setDepthCallback(PyObject *callable);

    /**
     * @brief Set the Python color camera callback.
     * 
     * @param callable A Python function.
     */
    void setColorCallback(PyObject *callable);

    /**
     * @brief Set the Python skeleton-tracker callback.
     * 
     * @param callable A Python function.
     */
    void setSkeletonCallback(PyObject *callable);

    /**
     * @brief Set the Python face-tracker callback.
     * 
     * @param callable A Python function.
     */
    void setFaceCallback(PyObject *callable);

    /**
     * @brief Set the Python hand-tracker callback.
     * 
     * @param callable A Python function.
     */
    void setHandsCallback(PyObject *callable);

    /**
     * @brief Set the Python user-tracker callback.
     * 
     * @param callable A Python function.
     */
    void setUserCallback(PyObject *callable);

    /**
     * @brief Set the Python gesture-tracker callback .
     * 
     * @param callable A Python function.
     */
    void setGestureCallback(PyObject *callable);

    /**
     * @brief Set the Python color camera callback 
     * 
     * @param callable A Python function.
     */
    void setIssueCallback(PyObject *callable);
};

/**
 * @brief General exception class.
 */
class NuitrackException : public std::exception
{
private:
    /// Message describing the error.
    std::string message;

public:
    /**
     * @brief Construct a new Nuitrack Exception object.
     * 
     * @param message String describing the error.
     */
    NuitrackException(std::string message);

    /**
     * @brief Returns the message with the error description.
     * 
     * @return const char* C-string with the error description.
     */
    const char *what() const throw();

    /**
     * @brief Destroy the Nuitrack Exception object.
     */
    ~NuitrackException() throw();
};

/**
 * @brief Exception used when pynuitrack fails to be initialized.
 */
class NuitrackInitFail : public NuitrackException
{
};

/**
 * @brief Translates a C++ exception to a Python exception.
 * 
 * @param e A C++ pynuitrack exception.
 */
void translateException(NuitrackException const &e);

/**
 * @brief Translates a Nuitrack error code into a error message.
 */
const char *exceptionType_str[] =
{
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