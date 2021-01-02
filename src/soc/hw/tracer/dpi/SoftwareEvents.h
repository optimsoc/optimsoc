#pragma once

#include <cstdint>
#include <cassert>
#include <string>
#include <map>
#include <vector>
#include <exception>

class ComposedEvent {
public:
    ComposedEvent(uint16_t _id, std::string _name) : id(_id), name(_name) {}
    void push_back(size_t bytes, std::string name) {
        fields.push_back(std::pair<size_t, std::string>(bytes, name));
    }
    void set_tsdl(char* _tsdl) {
        tsdl = _tsdl;
    }
    uint16_t id;
    std::string name;
    std::vector<std::pair<size_t, std::string> > fields;
    std::string tsdl;
};

class ComposedEventState {
public:
    ComposedEventState(ComposedEvent *ev) : mEvent(ev) {}
    bool trace(uint32_t value) {
        if (mEvent->fields.size() == 0) {
            return true;
        } else {
            mState.push_back(value);
            return (mState.size() == mEvent->fields.size());
        }
    }

    void emit(FILE* fh) {
        for (size_t f = 0; f < mEvent->fields.size(); f++) {
            assert(mEvent->fields[f].first <= 4);
            if (mEvent->fields[f].first > 0) {
                uint32_t value = mState[f];
                fwrite(&value, mEvent->fields[f].first, 1, fh);
            }
        }
        mState.clear();
    }
private:
    ComposedEvent *mEvent;
    std::vector<uint32_t> mState;
};

class ComposedEventRegistry {
public:
    void registerEvent(ComposedEvent *ev) {
        mEvents[ev->id] = ev;
    }
    ComposedEventState *generateState(uint16_t id) {
        if (mEvents.find(id) == mEvents.end()) {
            return nullptr;
        } else {
            return new ComposedEventState(mEvents[id]);
        }
    }
    static ComposedEventRegistry& instance() {
        static ComposedEventRegistry inst;
        return inst;
    }
    void writeMetadataEvents(FILE* fh) {
        for (std::map<uint16_t, ComposedEvent*>::iterator it = mEvents.begin();
                it != mEvents.end(); ++it) {
            ComposedEvent *ev = it->second;
            fprintf(fh, "event {\n");
            fprintf(fh, "    id = %d;\n", it->first);
            fprintf(fh, "    name = '%s';\n", ev->name.c_str());
            if ((ev->fields.size() > 0) || (ev->tsdl.length() > 0)) {
                fprintf(fh, "    fields := struct {\n");
                if (ev->tsdl.length() > 0) {
                    fprintf(fh, "%s\n", ev->tsdl.c_str());
                } else {
                    for (std::vector<std::pair<size_t, std::string> >::iterator it = ev->fields.begin();
                            it != ev->fields.end(); ++it) {
                        if (it->first == 0)
                            continue;

                        fprintf(fh, "        ");
                        if (it->first == 1) {
                            fprintf(fh, "uint8_t ");
                        } else if (it->first == 2) {
                            fprintf(fh, "uint16_t ");
                        } else {
                            fprintf(fh, "uint32_t ");
                        }

                        fprintf(fh, "%s;\n", it->second.c_str());
                    }
                }
                fprintf(fh, "    };\n");
            }
            fprintf(fh, "};\n");
        }
    }
private:
    ComposedEventRegistry() {}
    std::map<uint16_t, ComposedEvent*> mEvents;
};

class SoftwareState {
public:
    class UnknownEventException : public std::exception {};

    bool trace(uint16_t id, uint32_t value) {
        if (mEvents.find(id) == mEvents.end()) {
            ComposedEventState *es = ComposedEventRegistry::instance().generateState(id);
            if (es == nullptr) {
                throw UnknownEventException();
            }
            mEvents[id] = es;
        }

        return mEvents[id]->trace(value);
    }

    void emit(uint16_t id, FILE* fh) {
        assert(mEvents.find(id) != mEvents.end());
        mEvents[id]->emit(fh);
    }
private:
    std::map<uint16_t, ComposedEventState*> mEvents;
};

#define SOFTWARE_EVENT_BEGIN_DECL(id, name) \
    class event_##name##_Factory { \
        public: \
        event_##name##_Factory() { \
            ComposedEvent *ev = new ComposedEvent(id, #name); \
            ComposedEventRegistry::instance().registerEvent(ev);

#define SOFTWARE_EVENT_END_DECL(id, name) \
            } \
        }; \
        static event_##name##_Factory global_event_##name##_factory;


#define EVENT_FIELD(size, name) \
        ev->push_back(size, #name);

#define EVENT_TSDL(fields) \
        ev->set_tsdl((char*) fields);
