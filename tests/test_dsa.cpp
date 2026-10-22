#define HOSPITAL_DSA_TESTING
#include "../src/main.cpp"

#include <cassert>
#include <iostream>

int main() {
    {
        SinglyList<Visit> history;
        history.push_back({"2026-02-01", "Pediatrics", "First visit"});
        history.push_back({"2026-04-12", "Cardiology", "Second visit"});
        assert(history.size() == 2);
        auto copied_history = history;
        assert(copied_history.values().front().note == "First visit");
        history.clear();
        assert(history.size() == 0);
        assert(copied_history.size() == 2);
    }
    {
        StablePriorityQueue queue;
        queue.push({2, 0, "normal-first"});
        queue.push({1, 1, "emergency"});
        queue.push({2, 2, "normal-second"});
        QueueEntry next{};
        assert(queue.pop(next) && next.patient_id == "emergency");
        assert(queue.pop(next) && next.patient_id == "normal-first");
        assert(queue.pop(next) && next.patient_id == "normal-second");
        assert(!queue.pop(next));
    }
    {
        PatientIndex index;
        index.insert("P00002", 1);
        index.insert("P00001", 0);
        assert(index.find("P00001") && *index.find("P00001") == 0);
        assert(index.find("P00002") && *index.find("P00002") == 1);
        assert(index.find("P99999") == nullptr);
        index.clear();
        assert(index.find("P00001") == nullptr);
    }
    {
        std::vector<Appointment> appointments{
            {"A00002", "P00002", "Doctor", "2026-11-03", "10:00", false},
            {"A00001", "P00001", "Doctor", "2026-11-03", "09:00", false}
        };
        insertion_sort(appointments);
        assert(appointments[0].id == "A00001");
        assert(appointments[1].id == "A00002");
    }
    std::cout << "All integrated DSA tests passed.\n";
}
