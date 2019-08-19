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

namespace bp = boost::python;
namespace np = boost::python::numpy;

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
    PyObject* _pyColorCallback;

    np::dtype _dtUInt8 = np::dtype::get_builtin<uint8_t>();
    np::dtype _dtUInt16 = np::dtype::get_builtin<uint16_t>();

public:
    Nuitrack()
    {
        _pyDepthCallback = NULL;
        _dtUInt8 = np::dtype::get_builtin<uint8_t>();
        _dtUInt16 = np::dtype::get_builtin<uint16_t>();
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
        // _outputModeDepth = _depthSensor->getOutputMode();

        _colorSensor = tdv::nuitrack::ColorSensor::create();
        _colorSensor->connectOnNewFrame(std::bind(&Nuitrack::onNewRGBFrame, this, std::placeholders::_1));

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

        _skeletonTracker = tdv::nuitrack::SkeletonTracker::create();
        // Bind to event update skeleton tracker
        _skeletonTracker->connectOnUpdate(std::bind(&Nuitrack::onSkeletonUpdate, this, std::placeholders::_1));

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
            throw NuitrackException("Nuitrack update failed.");
            // std::cerr << "Nuitrack update failed (ExceptionType: " << e.type() << ")" << std::endl;
            // errorCode = EXIT_FAILURE;
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


    void onSkeletonUpdate(tdv::nuitrack::SkeletonData::Ptr userSkeletons)
    {
        return;
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

            std::cout << nCols << ", " << nRows << std::endl;

            np::ndarray npData = np::from_data(colorPtr, _dtUInt8,
                                            bp::make_tuple(nRows, nCols, 3),
                                            bp::make_tuple(nCols * 3 * sizeof(uint8_t), 3 * sizeof(uint8_t), sizeof(uint8_t)),
                                            bp::object());

            boost::python::call<void>(_pyColorCallback, npData.copy());
        }
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

    np::ndarray test()
    {
        int data[] = {1,2,3,4,5,6};
        np::dtype dt = np::dtype::get_builtin<int>();
        bp::tuple shape = bp::make_tuple(2,3);
        bp::tuple stride = bp::make_tuple(12,4);
        bp::object own;
        np::ndarray data_ex1 = np::from_data(data, dt, shape, stride, own);

        std::cout << "Single dimensional array ::" << std::endl
          << bp::extract<char const *>(bp::str(data_ex1)) << std::endl;


        return data_ex1.copy();
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
        // .def("greet", &World::greet)
        // .def("set", &World::set)
        // .def("many", &World::many)
        .def("init", &Nuitrack::init, nt_init_overloads(
            boost::python::arg("configPath")="", 
            "Path to the configuration file"
        ))
        .def("release", &Nuitrack::release)
        .def("set_depth_callback", &Nuitrack::setDepthCallback)
        .def("set_color_callback", &Nuitrack::setColorCallback)
        .def("update", &Nuitrack::update)
        .def("test", &Nuitrack::test)
    ;
};
