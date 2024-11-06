#pragma once
#include "ofThread.h"

class Events : public ofThread {
protected:
	struct Event {
		float time;
		function<void()> func;
	};

	float count = 0;
	float duration = 0;
	const int interval = 10;
	vector<Event> events;
	int currentEventIndex = 0;
	bool bFinished = false;

public:
	Events() {}

	Events(const Events& _Events) {
	}

	Events& operator=(Events&&) noexcept {
		cout << "Calld Move Constructor" << endl;
		return *this;
	}

	~Events() {
		waitForThread();
		stopThread();
		events.clear();
	}

	void start() {
		count = 0;
		currentEventIndex = 0;
		waitForThread();
		startThread();
	}

	void stop() {
		waitForThread();
		stopThread();
		bFinished = true;
	}

	void threadedFunction() {
		while (isThreadRunning()) {
			if (lock()) {

				count += 0.01;

				for (int i = currentEventIndex; i < events.size(); i++) {
					if (count >= events[i].time) {
						events[i].func();
						currentEventIndex++;
					}
				}

				if (duration == 0) {
					if (currentEventIndex >= events.size()) {
						stop();
					}
				}
				else {
					if (count >= duration) {
						stop();
					}
				}

				unlock();
				sleep(interval);
			}
		}
	}

	void addEvent(float time, function<void()> func) {
		Event e;
		e.time = time;
		e.func = func;
		events.push_back(e);
		sort(events.begin(),
			events.end(),
			[](const Event& a, const Event& b) {return (a.time < b.time); });
	}

	void setTotalDuration(float duration) {
		this->duration = duration;
	}

	float getDuration() const {
		return this->duration;
	}

	void clearEvents() {
		events.clear();
		currentEventIndex = 0;
	}

	float getCount() const {
		unique_lock<std::mutex> lock(mutex);
		return count;
	}

	bool isDone() const {
		return bFinished;
	}
};


class ofxEventsManager  {
protected:
	std::vector<Events*> events;

public:
	~ofxEventsManager() {
		events.clear();
	}

	void add(float time, Events* event) {
		event->setTotalDuration(time);
		events.push_back(event);
	}

	void start() {
		eraseElement();
		for (int i = 0; i < events.size(); i++) {
			events[i]->start();
			//cout << "Index of " << i << " : events state = " << events[i]->isDone() << endl;
		}
	}

	void start(int index) {
		eraseElement();
		events[index]->start();
	}

	void startAll() {
		for (int i = 0; i < events.size(); i++) {
			events[i]->start();
		}
	}

	void eraseElement() {
		auto itr = events.begin();
		while (itr != events.end()) {
			if (itr[0]->isDone() == true) {
				itr = events.erase(itr);
				//cout << "events was DELETED" << endl;
			}
			else {
				++itr;
			}
		}
		events.shrink_to_fit();
	}

	bool isRunning() {
		for (int i = 0; i < events.size(); i++) {
			if (events[i]->isThreadRunning()) {
				return true;
			}
		}
		return false;
	}
};