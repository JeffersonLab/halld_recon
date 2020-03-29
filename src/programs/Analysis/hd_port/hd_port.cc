
#include <JANA/JApplication.h>
#include <JANA/JCsvWriter.h>

#include <DAQ/JEventSourceGenerator_EVIO.h>

int main(int argc, char* argv[]) {
    JApplication app;
    app.Add(new JEventSourceGenerator_EVIO());
    //app.Add(new JCsvWriter<>);
    app.Run();
    return 0;
}

