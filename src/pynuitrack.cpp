#include <boost/python.hpp>
#include <boost/python/list.hpp>
#include <boost/python/extract.hpp>
#include <string>
#include <sstream>
#include <vector>

#include <nuitrack/Nuitrack.h>
#include <iomanip>

class NuiTrack
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

public:

    void apply(PyObject* callable)
    {
        // Invoke callable, passing a Python object which holds a reference to x
        boost::python::call<void>(callable);
    }

    int init(std::string configPath)
    {
        // Initialize Nuitrack
        try
        {
            tdv::nuitrack::Nuitrack::init(configPath);
        }
        catch (const tdv::nuitrack::Exception& e)
        {
            std::cerr << "Can not initialize Nuitrack (ExceptionType: " << e.type() << ")" << std::endl;
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

BOOST_PYTHON_MODULE(pynuitrack)
{
    class_<NuiTrack>("NuiTrack")
        // .def("greet", &World::greet)
        // .def("set", &World::set)
        // .def("many", &World::many)
        .def("init", &NuiTrack::init)
        .def("release", &NuiTrack::release)
        .def("apply", &NuiTrack::apply)
    ;
};
