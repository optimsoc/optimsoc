#include "Tracer.h"

#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>

Tracer::Tracer() : mEnabled(false), mNoCFull(false), mNoCTrace(NULL) {
}

void Tracer::init(bool nocfull) {
    mEnabled = true;
    mNoCFull = nocfull;

    std::time_t t = std::time(NULL);

    char folder[20];
    std::strftime(folder, 20, "ctf-%Y%m%d-%H%M%S", std::localtime(&t));
    mkdir(folder, 0777);

    mFolder = folder;

    std::string swfolder = mFolder + "/sw";
    mkdir(swfolder.c_str(), 0777);

    std::string metadata = swfolder + "/metadata";

    FILE *fh = fopen(metadata.c_str(), "w");
    fwrite(mSoftwareMetadata.c_str(), mSoftwareMetadata.length(), 1, fh);
    ComposedEventRegistry::instance().writeMetadataEvents(fh);
    fclose(fh);

    std::string trace = swfolder + "/trace";
    mSoftwareTrace = fopen(trace.c_str(), "w");
}

void Tracer::initNoC(int numLinks) {
    mNoCNumLinks = numLinks;
    mNoCBuffer.resize(numLinks);

    std::string folder = mFolder + "/noc";
    mkdir(folder.c_str(), 0777);

    std::string metadata = folder + "/metadata";

    FILE *fh = fopen(metadata.c_str(), "w");
    fwrite(mNoCMetadata.c_str(), mNoCMetadata.length(), 1, fh);
    fclose(fh);

    std::string trace = folder + "/trace";
    mNoCTrace = fopen(trace.c_str(), "w");
}

void Tracer::traceNoCPacket(int link, uint32_t flit, int last,
                            uint64_t timestamp) {
    if (!mNoCFull && (link % 2 == 0))
        return;

    mNoCBuffer[link].push_back(flit);

    if (last) {
        uint32_t header = mNoCBuffer[link][0];
        uint8_t cls = (header >> 24) & 0x7;
        int matched = 0;
        static const uint8_t CLASS_MPBUFFER = 0x0;
        static const uint8_t CLASS_MPBUFFER_CTRL = 0x7;
        static const uint8_t CLASS_MP = 0x2;

        if (cls == CLASS_MPBUFFER) {
            // mp buffer messages
            uint32_t payload_len = mNoCBuffer[link].size() - 1;
            matched = 1;

            emitNoCEventContext(link, 3, timestamp, header);
            fwrite(&mNoCBuffer[link][0], 4, 1, mNoCTrace);
            fwrite(&payload_len, 4, 1, mNoCTrace);
            for (size_t p = 1; p < mNoCBuffer[link].size(); p++) {
                fwrite(&mNoCBuffer[link][p], 4, 1, mNoCTrace);
            }
        } else if (cls == CLASS_MPBUFFER_CTRL) {
            // mp buffer control
            if ((mNoCBuffer[link][0] & 0x1) == 0) {
                // request
                matched = 1;
                emitNoCEventContext(link, 1, timestamp, header);
            } else {
                // response
                uint8_t status = (header >> 1) & 0x1;
                matched = 1;
                emitNoCEventContext(link, 2, timestamp, header);
                fwrite(&status, 1, 1, mNoCTrace);
            }
        } else if (cls == CLASS_MP) {
            // message passing
            uint8_t request = (header >> 15) & 0xf;
            static const uint8_t REQ_GET_EP       = 0x0;
            static const uint8_t RESP_GET_EP      = 0x1;
            static const uint8_t REQ_MSG_ALLOC    = 0x2;
            static const uint8_t RESP_MSG_ALLOC   = 0x3;
            static const uint8_t REQ_MSG_DATA     = 0x4;
            static const uint8_t REQ_MSG_COMPLETE = 0x5;

            if (request == REQ_GET_EP) {
                matched = 1;
                emitNoCEventContext(link, 4, timestamp, header);
                fwrite(&mNoCBuffer[link][1], 4, 1, mNoCTrace);
                fwrite(&mNoCBuffer[link][2], 4, 1, mNoCTrace);
            } else if (request == RESP_GET_EP) {
                matched = 1;
                if (mNoCBuffer[link][1] == 0xffffffff) {
                    emitNoCEventContext(link, 6, timestamp, header);
                } else {
                    emitNoCEventContext(link, 5, timestamp, header);
                    fwrite(&mNoCBuffer[link][1], 4, 1, mNoCTrace);
                }
            } else if (request == REQ_MSG_ALLOC) {
                matched = 1;
                emitNoCEventContext(link, 7, timestamp, header);
                fwrite(&mNoCBuffer[link][1], 4, 1, mNoCTrace);
                fwrite(&mNoCBuffer[link][2], 4, 1, mNoCTrace);
            } else if (request == RESP_MSG_ALLOC) {
                matched = 1;
                if (mNoCBuffer[link][1]) {
                    // ack
                    emitNoCEventContext(link, 8, timestamp, header);
                    fwrite(&mNoCBuffer[link][1], 4, 1, mNoCTrace);
                } else {
                    // nack
                    emitNoCEventContext(link, 9, timestamp, header);
                }
            } else if (request == REQ_MSG_DATA) {
                matched = 1;
                emitNoCEventContext(link, 10, timestamp, header);
                fwrite(&mNoCBuffer[link][1], 4, 1, mNoCTrace);
                fwrite(&mNoCBuffer[link][2], 4, 1, mNoCTrace);
                fwrite(&mNoCBuffer[link][3], 4, 1, mNoCTrace);
            } else if (request == REQ_MSG_COMPLETE) {
                matched = 1;
                emitNoCEventContext(link, 11, timestamp, header);
                fwrite(&mNoCBuffer[link][1], 4, 1, mNoCTrace);
                fwrite(&mNoCBuffer[link][2], 4, 1, mNoCTrace);
                fwrite(&mNoCBuffer[link][3], 4, 1, mNoCTrace);
            }
        }

        if (matched == 0) {
            uint32_t payload_len = mNoCBuffer[link].size() - 1;

            emitNoCEventContext(link, 0, timestamp, header);
            fwrite(&mNoCBuffer[link][0], 4, 1, mNoCTrace);
            fwrite(&payload_len, 4, 1, mNoCTrace);
            for (size_t p = 1; p < mNoCBuffer[link].size(); p++) {
                fwrite(&mNoCBuffer[link][p], 4, 1, mNoCTrace);
            }
        }

        fflush(mNoCTrace);
        mNoCBuffer[link].clear();
    }
}

void Tracer::emitNoCEventContext(int link, uint16_t id,
                                 uint64_t timestamp, uint32_t header) {
    if (!mEnabled)
        return;
    uint8_t src = (header >> 19) & 0x1f;
    uint8_t dest = (header >> 27) & 0x1f;
    uint16_t link_short = (uint16_t) link;

    fwrite(&timestamp, 8, 1, mNoCTrace);
    fwrite(&id, 2, 1, mNoCTrace);
    fwrite(&link_short, 2, 1, mNoCTrace);
    fwrite(&src, 1, 1, mNoCTrace);
    fwrite(&dest, 1, 1, mNoCTrace);
}

void Tracer::traceSoftware(uint64_t timestamp, uint16_t cpu, uint16_t id, uint32_t value) {
    if (!mEnabled)
        return;

    try {
        if (mSoftwareStates.find(cpu) == mSoftwareStates.end()) {
            mSoftwareStates[cpu] = new SoftwareState();
        }
        if (mSoftwareStates[cpu]->trace(id, value)) {
            fwrite(&timestamp, 8, 1, mSoftwareTrace);
            fwrite(&id, 2, 1, mSoftwareTrace);
            fwrite(&cpu, 2, 1, mSoftwareTrace);
            mSoftwareStates[cpu]->emit(id, mSoftwareTrace);
        }
    } catch(SoftwareState::UnknownEventException &) {
        uint16_t evid = 0;
        fwrite(&timestamp, 8, 1, mSoftwareTrace);
        fwrite(&evid, 2, 1, mSoftwareTrace);
        fwrite(&cpu, 2, 1, mSoftwareTrace);
        fwrite(&id, 2, 1, mSoftwareTrace);
        fwrite(&value, 4, 1, mSoftwareTrace);
    }
}


const std::string Tracer::mNoCMetadata =
        "/* CTF 1.8 */\n\n"
        "typealias integer {\n"
        "  size = 64;\n"
        "  signed = false;\n"
        "  align = 8;\n"
        "} := uint64_t;\n"
        "\n"
        "typealias integer {\n"
        "  size = 32;\n"
        "  signed = false;\n"
        "  align = 8;\n"
        "} := uint32_t;\n"
        "\n"
        "typealias integer {\n"
        "    size = 16;\n"
        "    signed = false;\n"
        "    align = 8;\n"
        "} := uint16_t;\n"
        "\n"
        "typealias integer {\n"
        "    size = 8;\n"
        "    signed = false;\n"
        "    align = 8;\n"
        "} := uint8_t;\n"
        "\n"
        "trace {\n"
        "    major = 1;\n"
        "    minor = 8;\n"
        "    byte_order = le;\n"
        "};\n"
        "\n"
        "stream {\n"
        "    event.header := struct {\n"
        "        uint64_t timestamp;\n"
        "        uint16_t id;\n"
        "    };\n"
        "    event.context := struct {\n"
        "        uint16_t link;\n"
        "     uint8_t src;\n"
        "     uint8_t dest;\n"
        "    };\n"
        "};\n"
        "\n"
        "event {\n"
        "    id = 0;\n"
        "    name = 'unknown';\n"
        "    fields := struct {\n"
        "        uint32_t header;\n"
        "     uint32_t length;\n"
        "     uint32_t payload[length];\n"
        "    };\n"
        "};\n"
        "\n"
        "event {\n"
        "    id = 1;\n"
        "    name = 'mpbuffer_control_req';\n"
        "};\n"
        "\n"
        "event {\n"
        "    id = 2;\n"
        "    name = 'mpbuffer_control_resp';\n"
        "    fields := struct {\n"
        "        uint8_t status;\n"
        "    };\n"
        "};\n"
        "\n"
        "event {\n"
        "    id = 3;\n"
        "    name = 'mpbuffer_message';\n"
        "    fields := struct {\n"
        "        uint32_t header;\n"
        "     uint32_t length;\n"
        "     uint32_t payload[length];\n"
        "    };\n"
        "};\n"
        "\n"
        "event {\n"
        "    id = 4;\n"
        "    name = 'mp_getep_req';\n"
        "    fields := struct {\n"
        "        uint32_t node;\n"
        "        uint32_t port;\n"
        "    };\n"
        "};\n"
        "event {\n"
        "    id = 5;\n"
        "    name = 'mp_getep_resp_ack';\n"
        "    fields := struct {\n"
        "        uint32_t handle;\n"
        "    };\n"
        "};\n"
        "event {\n"
        "    id = 6;\n"
        "    name = 'mp_getep_resp_nack';\n"
        "};\n"
        "event {\n"
        "    id = 7;\n"
        "    name = 'mp_msgalloc_req';\n"
        "    fields := struct {\n"
        "        uint32_t handle;\n"
        "        uint32_t size;\n"
        "    };\n"
        "};\n"
        "event {\n"
        "    id = 8;\n"
        "    name = 'mp_msgalloc_resp_ack';\n"
        "    fields := struct {\n"
        "        uint32_t ptr;\n"
        "    };\n"
        "};\n"
        "event {\n"
        "    id = 9;\n"
        "    name = 'mp_msgalloc_resp_nack';\n"
        "};\n"
        "event {\n"
        "    id = 10;\n"
        "    name = 'mp_msgdata';\n"
        "    fields := struct {\n"
        "        uint32_t handle;\n"
        "        uint32_t address;\n"
        "        uint32_t offset;\n"
        "    };\n"
        "};\n"
        "event {\n"
        "    id = 11;\n"
        "    name = 'mp_msgcomplete';\n"
        "    fields := struct {\n"
        "        uint32_t handle;\n"
        "        uint32_t address;\n"
        "        uint32_t size;\n"
        "    };\n"
        "};\n"
        "\n";
