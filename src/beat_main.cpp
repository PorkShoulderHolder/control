//
// Created by Sam Royston on 9/10/17.
//

#include "argparse.hpp"
#include <csignal>
#include "state.h"
#include "network_manager.h"
#include "midireader/MidiFile.h"
#include <sys/time.h>
#include <chrono>
#include <unordered_map>
#include "bot.h"
#include "utils.h"

const char *M_COMMAND_STR_[4] = { "r:::0", "l:::0", "r:::1", "l:::1" };

using namespace std::chrono;

void exiting(int i){
    State *S = State::shared_instance();
    for( Bot *b : S->devices ){
        std::vector<MOTOR> off;
        off.push_back(M_RIGHT_OFF);
        off.push_back(M_LEFT_OFF);
        b->apply_motor_commands(off);
        delete b;
    }
    NetworkManager *manager = new NetworkManager();
    std::string msg = "{\"completed\":1}";
    char buffer[1024];
    std::string brain_host = "127.0.0.1";
    manager->send_tcp((char*)brain_host.c_str(), (char*)msg.c_str(), 9999, buffer);
    std::cout << "\n --- bye :^) --- \n" << std::endl;
    std::exit(0);
}

void setTempo(MidiFile& midifile, int index, double& tempo) {
    double newtempo = 0.0;
    static int count = 0;
    count++;

    MidiEvent& mididata = midifile[0][index];

    int microseconds = 0;
    microseconds = microseconds | (mididata[3] << 16);
    microseconds = microseconds | (mididata[4] << 8);
    microseconds = microseconds | (mididata[5] << 0);

    newtempo = 60.0 / microseconds * 1000000.0;
    if (count <= 1) {
        tempo = newtempo;
    } else if (tempo != newtempo) {
        cout << "; WARNING: change of tempo from " << tempo
        << " to " << newtempo << " ignored" << endl;
    }
}



void midi_loop(std::string filename, bool repeat){
    State *S = State::shared_instance(false);
    MidiFile midifile;
    midifile.read(filename);

    cout << "TPQ: " << midifile.getTicksPerQuarterNote() << endl;
    cout << "TRACKS: " << midifile.getTrackCount() << endl;

    if(midifile.getTrackCount() != S->devices.size() * 2){
        cout << "More tracks than bot motors " << midifile.getTrackCount() << endl;
    }
    midifile.absoluteTicks();
    midifile.joinTracks();
    // midifile.getTrackCount() will now return "1", but original
    // track assignments can be seen in .track field of MidiEvent.

    cout << "TICK    DELTA   TRACK   MIDI MESSAGE\n";
    cout << "____________________________________\n";

    MidiEvent* mev;
    int deltatick;
    int command = 0;
    int key = 0;
    int vel = 0;
    double tempo = 60.0;
    double offtime = 0.0;
    vector<double> ontimes(128);
    vector<int> onvelocities(128);
    vector<pair<double, int>> starts;
    vector<pair<double, int>> ends;

    int motor_count = (int)S->devices.size() * 2;
    int i;

    for (i=0; i<128; i++) {
        ontimes[i] = -1.0;
        onvelocities[i] = -1;
    }
    for (int i=0; i<midifile.getNumEvents(0); i++) {
        command = midifile[0][i][0] & 0xf0;
        if (command == 0x90 && midifile[0][i][2] != 0) {
            // store note-on velocity and time
            key = midifile[0][i][1];
            vel = midifile[0][i][2];
            ontimes[key] = midifile[0][i].tick * 60.0 / tempo /
                           midifile.getTicksPerQuarterNote();
            onvelocities[key] = vel;
        } else if (command == 0x90 || command == 0x80) {
            // note off command write to output
            key = midifile[0][i][1];
            offtime = midifile[0][i].tick * 60.0 /
                      midifile.getTicksPerQuarterNote() / tempo;
            cout << "note\t" << ontimes[key]
            << "\t" << offtime - ontimes[key]
            << "\t" << key << "\t" << onvelocities[key] << endl;

            starts.push_back(std::pair<double, int>(ontimes[key], key % motor_count));
            ends.push_back(std::pair<double, int>(offtime, key % motor_count));

            onvelocities[key] = -1;
            ontimes[key] = -1.0;
        }

        // check for tempo indication
        if (midifile[0][i][0] == 0xff &&
            midifile[0][i][1] == 0x51) {
            setTempo(midifile, i, tempo);
        }
    }
    int k = 0;
    int l  = 0;
    double current_time = duration_cast< milliseconds >(
            system_clock::now().time_since_epoch()).count();

    NetworkManager *manager = new NetworkManager();

    while(l < starts.size() && k < ends.size()){
        double t = duration_cast< milliseconds >(system_clock::now().time_since_epoch()).count();
        if(starts[l].first * 1000.0 <  t - current_time){
            int bot_index = starts[l].second / 2;
            Bot *bot = S->devices[bot_index];
            int cint = starts[l].second % 2 ? M_LEFT_ON : M_RIGHT_ON;
            std::cout << starts[l].first << " " << k << " " << k << std::endl;
            manager->send_udp(bot->host, (char *) M_COMMAND_STR_[cint], bot->port);
            manager->send_udp(bot->host, (char *) M_COMMAND_STR_[cint], bot->port);
            l++;
        }
        if(ends[k].first * 1000.0 < t - current_time){
            int bot_index = ends[k].second / 2;
            Bot *bot = S->devices[bot_index];
            int cint = ends[k].second % 2 ? M_LEFT_OFF : M_RIGHT_OFF;
            manager->send_udp(bot->host, (char *) M_COMMAND_STR_[cint], bot->port);
            manager->send_udp(bot->host, (char *) M_COMMAND_STR_[cint], bot->port);
            k++;
        }
    }
    for(Bot *bot : S->devices){
        manager->send_udp(bot->host, (char *) M_COMMAND_STR_[M_RIGHT_OFF], bot->port);
        manager->send_udp(bot->host, (char *) M_COMMAND_STR_[M_LEFT_OFF], bot->port);
    }
}

void timer_loop(std::vector<string>){
    State *S = State::shared_instance(false);
    while(true) {

    }
}

int main(int argc, const char** argv){

    system("gtimeout 2 ./mdns-mod -B");

    ArgumentParser parser;
    parser.addArgument("-m", "--midi", '*');
    parser.addArgument("-t", "--timer");
    parser.parse((size_t)argc, argv);

    std::signal(SIGINT, exiting);
    srand (static_cast <unsigned> (time(0)));
    if (parser.exists("midi")){
        vector<string> args = parser.retrieve<std::vector<std::string> >("midi");
        std::string filename = args[0];
        midi_loop(filename, parser.exists("repeat"));
    }
    else if(parser.exists("timer")){
        vector<string> args = parser.retrieve<std::vector<std::string> >("timer");
        timer_loop(args);
    }

    return 0;
}