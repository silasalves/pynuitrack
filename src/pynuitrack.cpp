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
    tdv::nuitrack::HandTracker::Ptr handTracker;

    tdv::nuitrack::OutputMode _outputMode;
	tdv::nuitrack::DepthSensor::Ptr _depthSensor;
	tdv::nuitrack::ColorSensor::Ptr _colorSensor;
	tdv::nuitrack::UserTracker::Ptr _userTracker;
	tdv::nuitrack::SkeletonTracker::Ptr _skeletonTracker;
	tdv::nuitrack::HandTracker::Ptr _handTracker;
	tdv::nuitrack::GestureRecognizer::Ptr _gestureRecognizer;
	tdv::nuitrack::IssuesData::Ptr _issuesData;

public:

    void apply(PyObject* callable)
    {
        // Invoke callable, passing a Python object which holds a reference to x
        boost::python::call<void>(callable);
    }

    int init(std::string configPath = "")
    {
        throw NuitrackException("Could not initialize Nuitrack");
        // Initialize Nuitrack
        try
        {
            tdv::nuitrack::Nuitrack::init(configPath);
        }
        catch (const tdv::nuitrack::Exception& e)
        {
            // std::cerr << "Can not initialize Nuitrack (ExceptionType: " << e.type() << ")" << std::endl;
            throw NuitrackException("Could not initialize Nuitrack");
            return -1;
        }

        handTracker = tdv::nuitrack::HandTracker::create();

        // Connect onHandUpdate callback to receive hand tracking data
        handTracker->connectOnUpdate(onHandUpdate);

        // Start Nuitrack
        try
        {
            tdv::nuitrack::Nuitrack::run();
        }
        catch (const tdv::nuitrack::Exception& e)
        {
            std::cerr << "Can not start Nuitrack (ExceptionType: " << e.type() << ")" << std::endl;
            return EXIT_FAILURE;
        }

        int errorCode = EXIT_SUCCESS;
        int a = 100;
        while (a--)
        {
            try
            {
                // Wait for new hand tracking data
                tdv::nuitrack::Nuitrack::waitUpdate(handTracker);
            }
            catch (tdv::nuitrack::LicenseNotAcquiredException& e)
            {
                std::cerr << "LicenseNotAcquired exception (ExceptionType: " << e.type() << ")" << std::endl;
                errorCode = EXIT_FAILURE;
                break;
            }
            catch (const tdv::nuitrack::Exception& e)
            {
                std::cerr << "Nuitrack update failed (ExceptionType: " << e.type() << ")" << std::endl;
                errorCode = EXIT_FAILURE;
            }
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
};

using namespace boost::python;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(nt_init_overloads, Nuitrack::init, 0, 1)

BOOST_PYTHON_MODULE(pynuitrack)
{
    register_exception_translator<NuitrackException>(&translateException);
    register_exception_translator<NuitrackInitFail>(&translateException);

    class_<Nuitrack>("Nuitrack")
        // .def("greet", &World::greet)
        // .def("set", &World::set)
        // .def("many", &World::many)
        .def("init", &Nuitrack::init, nt_init_overloads(
            boost::python::arg("configPath")="", 
            "Path to the configuration file"
        ))
        .def("release", &Nuitrack::release)
        .def("apply", &Nuitrack::apply);
};
