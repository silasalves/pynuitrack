#include "pynuitrack.hpp"

#include <boost/python.hpp>
#include <boost/python/list.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/exception_translator.hpp>

#include <string>
#include <sstream>
#include <vector>

#include <nuitrack/Nuitrack.h>
#include <iomanip>

// struct NuitrackInitFail : std::exception
// {
//   char const* what() throw() { return "One of my exceptions"; }
// };

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
    // Use the Python 'C' API to set up an exception object
    PyErr_SetString(PyExc_RuntimeError, e.what());
}

class Nuitrack
{
    // void set(std::string msg) { mMsg = msg; }
    // void many(boost::python::list msgs) {
    //     long l = len(msgs);
    //     std::stringstream ss;
    //     for (long i = 0; i<l; ++i) {
    //         if (i>0) ss << ", ";
    //         std::string s = boost::python::extract<std::string>(msgs[i]);
    //         ss << s;
    //     }
    //     mMsg = ss.str();
    // }
    // std::string greet() { return mMsg; }
    // std::string mMsg;
private:
    // tdv::nuitrack::HandTracker::Ptr handTracker;

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

public:
    Nuitrack()
    {
        _pyDepthCallback = NULL;
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

        // _colorSensor = tdv::nuitrack::ColorSensor::create();
        // _colorSensor->connectOnNewFrame(this->onNewRGBFrame);

        // _handTracker = tdv::nuitrack::HandTracker::create();
        // _handTracker->connectOnUpdate(onHandUpdate);


        // _outputMode = _depthSensor->getOutputMode();
        // OutputMode colorOutputMode = _colorSensor->getOutputMode();
        // if (colorOutputMode.xres > _outputMode.xres)
        // 	_outputMode.xres = colorOutputMode.xres;
        // if (colorOutputMode.yres > _outputMode.yres)
        // 	_outputMode.yres = colorOutputMode.yres;

        // _width = _outputMode.xres;
        // _height = _outputMode.yres;

        // _userTracker = UserTracker::create();
        // // Bind to event update user tracker
        // _userTracker->connectOnUpdate(std::bind(&NuitrackGLSample::onUserUpdate, this, std::placeholders::_1));

        // _skeletonTracker = SkeletonTracker::create();
        // // Bind to event update skeleton tracker
        // _skeletonTracker->connectOnUpdate(std::bind(&NuitrackGLSample::onSkeletonUpdate, this, std::placeholders::_1));

        // _handTracker = HandTracker::create();
        // // Bind to event update Hand tracker
        // _handTracker->connectOnUpdate(std::bind(&NuitrackGLSample::onHandUpdate, this, std::placeholders::_1));

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
            throw NuitrackException("Could not run Nuitrack"); // e.type());
        }

        // int errorCode = EXIT_SUCCESS;
        // int a = 100;
        // while (a--)
        // {
        //     try
        //     {
        //         // Wait for new hand tracking data
        //         tdv::nuitrack::Nuitrack::waitUpdate(_handTracker);
        //     }
        //     catch (tdv::nuitrack::LicenseNotAcquiredException& e)
        //     {
        //         std::cerr << "LicenseNotAcquired exception (ExceptionType: " << e.type() << ")" << std::endl;
        //         errorCode = EXIT_FAILURE;
        //         break;
        //     }
        //     catch (const tdv::nuitrack::Exception& e)
        //     {
        //         std::cerr << "Nuitrack update failed (ExceptionType: " << e.type() << ")" << std::endl;
        //         errorCode = EXIT_FAILURE;
        //     }
        // }
    }

    void set_depth_callback(PyObject* callable)
    {
        // Invoke callable, passing a Python object which holds a reference to x
        // boost::python::call<void>(callable);
        _pyDepthCallback = callable;
    }

    void onNewDepthFrame(tdv::nuitrack::DepthFrame::Ptr frame)
    {
        if (_pyDepthCallback != NULL)
        {
            const uint16_t* depthPtr = frame->getData();
            float nCols = frame->getCols();
	        float nRows = frame->getRows();

            boost::python::call<void>(_pyDepthCallback);
        }
    }

    void onNewRGBFrame(tdv::nuitrack::RGBFrame::Ptr frame)
    {
        return;
    }


    // Callback for the hand data update event
    static void onHandUpdate(tdv::nuitrack::HandTrackerData::Ptr handData)
    {
        if (!handData)
        {
            // No hand data
            std::cout << "No hand data" << std::endl;
            return;
        }

        auto userHands = handData->getUsersHands();
        if (userHands.empty())
        {
            // No user hands
            return;
        }

        auto rightHand = userHands[0].rightHand;
        if (!rightHand)
        {
            // No right hand
            std::cout << "Right hand of the first user is not found" << std::endl;
            return;
        }

        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Right hand position: "
                    "x = " << rightHand->xReal << ", "
                    "y = " << rightHand->yReal << ", "
                    "z = " << rightHand->zReal << std::endl;
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
    register_exception_translator<NuitrackException>(&translateException);
    register_exception_translator<NuitrackInitFail>(&translateException);

    class_<Nuitrack>("Nuitrack", boost::python::init<>())
        // .def("greet", &World::greet)
        // .def("set", &World::set)
        // .def("many", &World::many)
        .def("init", &Nuitrack::init, nt_init_overloads(
            boost::python::arg("configPath")="", 
            "Path to the configuration file"
        ))
        .def("release", &Nuitrack::release)
    ;
};