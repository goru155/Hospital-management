#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace dsa {

template <typename T>
class SinglyList {
    struct Node {
        T value;
        Node* next = nullptr;
        explicit Node(T item) : value(std::move(item)) {}
    };
    Node* head_ = nullptr;
    Node* tail_ = nullptr;

public:
    SinglyList() = default;
    SinglyList(const SinglyList& other) {
        for (const auto& value : other.values()) push_back(value);
    }
    SinglyList& operator=(const SinglyList& other) {
        if (this != &other) {
            clear();
            for (const auto& value : other.values()) push_back(value);
        }
        return *this;
    }
    ~SinglyList() { clear(); }

    void push_back(T value) {
        Node* node = new Node(std::move(value));
        if (tail_) tail_->next = node;
        else head_ = node;
        tail_ = node;
    }

    bool erase(const T& value) {
        Node* previous = nullptr;
        for (Node* node = head_; node; previous = node, node = node->next) {
            if (node->value == value) {
                if (previous) previous->next = node->next;
                else head_ = node->next;
                if (tail_ == node) tail_ = previous;
                delete node;
                return true;
            }
        }
        return false;
    }

    std::vector<T> values() const {
        std::vector<T> result;
        for (Node* node = head_; node; node = node->next) result.push_back(node->value);
        return result;
    }

    std::size_t size() const {
        std::size_t count = 0;
        for (Node* node = head_; node; node = node->next) ++count;
        return count;
    }

    void clear() {
        while (head_) {
            Node* next = head_->next;
            delete head_;
            head_ = next;
        }
        tail_ = nullptr;
    }
};

struct QueueEntry {
    int priority;
    std::size_t arrival;
    std::string patient_id;
};

class StablePriorityQueue {
    std::vector<QueueEntry> heap_;

    static bool before(const QueueEntry& a, const QueueEntry& b) {
        if (a.priority != b.priority) return a.priority > b.priority;
        return a.arrival > b.arrival;
    }

public:
    void push(QueueEntry entry) {
        heap_.push_back(std::move(entry));
        std::push_heap(heap_.begin(), heap_.end(), before);
    }

    bool pop(QueueEntry& entry) {
        if (heap_.empty()) return false;
        std::pop_heap(heap_.begin(), heap_.end(), before);
        entry = std::move(heap_.back());
        heap_.pop_back();
        return true;
    }

    bool empty() const { return heap_.empty(); }
};

class PatientIndex {
    struct Node {
        std::string id;
        std::size_t position;
        Node* left = nullptr;
        Node* right = nullptr;
        Node(std::string key, std::size_t index) : id(std::move(key)), position(index) {}
    };
    Node* root_ = nullptr;

    static Node* insert(Node* node, const std::string& id, std::size_t position) {
        if (!node) return new Node(id, position);
        if (id < node->id) node->left = insert(node->left, id, position);
        else if (node->id < id) node->right = insert(node->right, id, position);
        else node->position = position;
        return node;
    }

    static void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    PatientIndex() = default;
    PatientIndex(const PatientIndex&) = delete;
    PatientIndex& operator=(const PatientIndex&) = delete;
    ~PatientIndex() { clear(); }

    void insert(const std::string& id, std::size_t position) {
        root_ = insert(root_, id, position);
    }

    const std::size_t* find(const std::string& id) const {
        Node* node = root_;
        while (node) {
            if (id < node->id) node = node->left;
            else if (node->id < id) node = node->right;
            else return &node->position;
        }
        return nullptr;
    }

    void clear() {
        destroy(root_);
        root_ = nullptr;
    }
};

template <typename T>
void insertion_sort(std::vector<T>& values) {
    for (std::size_t i = 1; i < values.size(); ++i) {
        T value = values[i];
        std::size_t j = i;
        while (j > 0 && value < values[j - 1]) {
            values[j] = values[j - 1];
            --j;
        }
        values[j] = std::move(value);
    }
}

} // namespace dsa

using namespace dsa;

struct Visit {
    std::string date;
    std::string department;
    std::string note;
    bool operator<(const Visit& other) const { return date < other.date; }
};

struct Patient {
    std::string id;
    std::string name;
    std::string contact;
    std::string department;
    int age = 0;
    SinglyList<Visit> visits;
};

struct Appointment {
    std::string id;
    std::string patient_id;
    std::string doctor;
    std::string date;
    std::string time;
    bool cancelled = false;
    bool operator<(const Appointment& other) const {
        return date != other.date ? date < other.date : time < other.time;
    }
};

class Hospital {
    const std::vector<std::string> doctors_{
        "Dr. A. Rao - Cardiology",
        "Dr. M. Shah - Pediatrics",
        "Dr. S. Iyer - General Medicine"
    };
    std::vector<Patient> patients_;
    PatientIndex patient_index_;
    std::vector<Appointment> appointments_;
    std::vector<QueueEntry> waiting_;
    fs::path data_file_ = fs::path("data") / "hospital.db";
    std::size_t next_patient_id_ = 1;
    std::size_t next_appointment_id_ = 1;
    std::size_t arrival_order_ = 0;

    static std::string make_id(char prefix, std::size_t number) {
        std::ostringstream output;
        output << prefix << std::setw(5) << std::setfill('0') << number;
        return output.str();
    }

    static bool read_quoted(std::istream& input, std::string& value) {
        return static_cast<bool>(input >> std::quoted(value));
    }

    void load() {
        if (!fs::exists(data_file_)) return;
        std::ifstream input(data_file_);
        std::string tag;
        std::size_t patient_counter = 0, appointment_counter = 0, count = 0;
        std::vector<Patient> patients;
        std::vector<Appointment> appointments;

        if (!(input >> tag) || tag != "HOSPITAL_V1" ||
            !(input >> tag >> patient_counter >> appointment_counter) || tag != "COUNTERS" ||
            !(input >> tag >> count) || tag != "PATIENTS")
            throw std::runtime_error("Data file is damaged or has an unsupported version.");

        for (std::size_t i = 0; i < count; ++i) {
            Patient patient;
            std::size_t visits = 0;
            if (!read_quoted(input, patient.id) || !read_quoted(input, patient.name) ||
                !(input >> patient.age) || !read_quoted(input, patient.contact) ||
                !read_quoted(input, patient.department) || !(input >> visits))
                throw std::runtime_error("Invalid patient record in data file.");
            for (std::size_t j = 0; j < visits; ++j) {
                Visit visit;
                if (!read_quoted(input, visit.date) || !read_quoted(input, visit.department) ||
                    !read_quoted(input, visit.note))
                    throw std::runtime_error("Invalid visit in data file.");
                patient.visits.push_back(std::move(visit));
            }
            patients.push_back(std::move(patient));
        }

        if (!(input >> tag >> count) || tag != "APPOINTMENTS")
            throw std::runtime_error("Invalid appointment section in data file.");
        for (std::size_t i = 0; i < count; ++i) {
            Appointment appointment;
            if (!read_quoted(input, appointment.id) || !read_quoted(input, appointment.patient_id) ||
                !read_quoted(input, appointment.doctor) || !read_quoted(input, appointment.date) ||
                !read_quoted(input, appointment.time) || !(input >> appointment.cancelled))
                throw std::runtime_error("Invalid appointment record in data file.");
            appointments.push_back(std::move(appointment));
        }

        patients_ = std::move(patients);
        appointments_ = std::move(appointments);
        next_patient_id_ = std::max<std::size_t>(patient_counter, 1);
        next_appointment_id_ = std::max<std::size_t>(appointment_counter, 1);
        rebuild_patient_index();
    }

    void save() const {
        fs::create_directories(data_file_.parent_path());
        fs::path temporary = data_file_.string() + ".tmp";
        fs::path backup = data_file_.string() + ".bak";
        {
            std::ofstream output(temporary, std::ios::trunc);
            if (!output) throw std::runtime_error("Could not create temporary data file.");
            output << "HOSPITAL_V1\nCOUNTERS " << next_patient_id_ << ' '
                   << next_appointment_id_ << "\nPATIENTS " << patients_.size() << '\n';
            for (const auto& patient : patients_) {
                output << std::quoted(patient.id) << ' ' << std::quoted(patient.name) << ' '
                       << patient.age << ' ' << std::quoted(patient.contact) << ' '
                       << std::quoted(patient.department) << ' ' << patient.visits.size() << '\n';
                for (const auto& visit : patient.visits.values())
                    output << std::quoted(visit.date) << ' ' << std::quoted(visit.department)
                           << ' ' << std::quoted(visit.note) << '\n';
            }
            output << "APPOINTMENTS " << appointments_.size() << '\n';
            for (const auto& appointment : appointments_)
                output << std::quoted(appointment.id) << ' ' << std::quoted(appointment.patient_id)
                       << ' ' << std::quoted(appointment.doctor) << ' '
                       << std::quoted(appointment.date) << ' ' << std::quoted(appointment.time)
                       << ' ' << appointment.cancelled << '\n';
            output.flush();
            if (!output) throw std::runtime_error("Could not write data file.");
        }

        bool had_previous = fs::exists(data_file_);
        if (had_previous) {
            std::error_code error;
            fs::remove(backup, error);
            error.clear();
            fs::rename(data_file_, backup, error);
            if (error) throw std::runtime_error("Could not preserve the previous data file.");
        }
        std::error_code error;
        fs::rename(temporary, data_file_, error);
        if (error) {
            if (had_previous) {
                std::error_code restore_error;
                fs::rename(backup, data_file_, restore_error);
            }
            throw std::runtime_error("Could not save data file.");
        }
    }

    template <typename Change>
    void save_change(Change change) {
        auto old_patients = patients_;
        auto old_appointments = appointments_;
        auto old_waiting = waiting_;
        const auto old_patient_counter = next_patient_id_;
        const auto old_appointment_counter = next_appointment_id_;
        try {
            change();
            save();
        } catch (...) {
            patients_ = std::move(old_patients);
            appointments_ = std::move(old_appointments);
            waiting_ = std::move(old_waiting);
            next_patient_id_ = old_patient_counter;
            next_appointment_id_ = old_appointment_counter;
            rebuild_patient_index();
            throw;
        }
    }

    void rebuild_patient_index() {
        patient_index_.clear();
        for (std::size_t i = 0; i < patients_.size(); ++i)
            patient_index_.insert(patients_[i].id, i);
    }

    Patient* find_patient(const std::string& id) {
        const std::size_t* position = patient_index_.find(id);
        return position && *position < patients_.size() ? &patients_[*position] : nullptr;
    }

    static std::string prompt(const std::string& label) {
        std::cout << label;
        std::string value;
        if (!std::getline(std::cin, value)) throw std::runtime_error("Input closed.");
        return value;
    }

    static int number(const std::string& label, int minimum, int maximum) {
        while (true) {
            std::istringstream input(prompt(label));
            int value;
            char extra;
            if (input >> value && !(input >> extra) && value >= minimum && value <= maximum)
                return value;
            std::cout << "[ERROR] Enter a number from " << minimum << " to " << maximum << ".\n";
        }
    }

    static bool valid_date(const std::string& date) {
        if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
        for (std::size_t i = 0; i < date.size(); ++i)
            if (i != 4 && i != 7 && !std::isdigit(static_cast<unsigned char>(date[i]))) return false;
        int month = std::stoi(date.substr(5, 2));
        int day = std::stoi(date.substr(8, 2));
        int year = std::stoi(date.substr(0, 4));
        if (year < 1 || month < 1 || month > 12 || day < 1) return false;
        const int month_days[] = {
            31, (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28,
            31, 30, 31, 30, 31, 31, 30, 31, 30, 31
        };
        return day <= month_days[month - 1];
    }

    static bool valid_time(const std::string& time) {
        if (time.size() != 5 || time[2] != ':') return false;
        if (!std::isdigit(static_cast<unsigned char>(time[0])) ||
            !std::isdigit(static_cast<unsigned char>(time[1])) ||
            !std::isdigit(static_cast<unsigned char>(time[3])) ||
            !std::isdigit(static_cast<unsigned char>(time[4]))) return false;
        int hour = std::stoi(time.substr(0, 2));
        int minute = std::stoi(time.substr(3, 2));
        return hour < 24 && minute < 60;
    }

    static bool confirm(const std::string& question) {
        return prompt(question + " (y/N): ") == "y";
    }

    void print_patient(const Patient& patient) const {
        std::cout << patient.id << " | " << patient.name << " | age " << patient.age
                  << " | " << patient.contact << " | " << patient.department << '\n';
        for (const auto& visit : patient.visits.values())
            std::cout << "  Visit: " << visit.date << " | " << visit.department
                      << " | " << visit.note << '\n';
    }

    void patients_menu() {
        while (true) {
            std::cout << "\n-- Patients --\n1 Register  2 Search  3 List  4 Update  5 Delete  0 Back\n";
            switch (number("Choice: ", 0, 5)) {
            case 0: return;
            case 1: {
                Patient patient;
                patient.id = make_id('P', next_patient_id_);
                patient.name = prompt("Name: ");
                patient.age = number("Age (0-120): ", 0, 120);
                patient.contact = prompt("Contact: ");
                patient.department = prompt("Department: ");
                if (patient.name.empty() || patient.contact.empty() || patient.department.empty()) {
                    std::cout << "[ERROR] All fields are required.\n";
                    break;
                }
                save_change([&] {
                    patients_.push_back(std::move(patient));
                    patient_index_.insert(patients_.back().id, patients_.size() - 1);
                    ++next_patient_id_;
                });
                std::cout << "[OK] Patient registered.\n";
                break;
            }
            case 2: {
                std::string query = prompt("Patient ID or name: ");
                bool found = false;
                if (Patient* patient = find_patient(query)) {
                    print_patient(*patient);
                    found = true;
                } else {
                    for (const auto& patient : patients_) {
                        if (patient.name.find(query) != std::string::npos) {
                            print_patient(patient);
                            found = true;
                        }
                    }
                }
                if (!found) std::cout << "[WARN] No matching patient.\n";
                break;
            }
            case 3:
                if (patients_.empty()) std::cout << "No patients registered.\n";
                for (const auto& patient : patients_) print_patient(patient);
                break;
            case 4: {
                Patient* patient = find_patient(prompt("Patient ID: "));
                if (!patient) {
                    std::cout << "[WARN] Patient not found.\n";
                    break;
                }
                std::string name = prompt("New name (blank keeps current): ");
                std::string contact = prompt("New contact (blank keeps current): ");
                std::string department = prompt("New department (blank keeps current): ");
                save_change([&] {
                    if (!name.empty()) patient->name = name;
                    if (!contact.empty()) patient->contact = contact;
                    if (!department.empty()) patient->department = department;
                });
                std::cout << "[OK] Patient updated.\n";
                break;
            }
            case 5: {
                std::string id = prompt("Patient ID to delete: ");
                const std::size_t* position = patient_index_.find(id);
                if (!position || *position >= patients_.size()) {
                    std::cout << "[WARN] Patient not found.\n";
                    break;
                }
                const std::size_t index = *position;
                if (!confirm("Delete this patient and their appointments?")) break;
                save_change([&] {
                    patients_.erase(patients_.begin() + static_cast<std::ptrdiff_t>(index));
                    rebuild_patient_index();
                    appointments_.erase(std::remove_if(appointments_.begin(), appointments_.end(),
                        [&](const Appointment& appointment) { return appointment.patient_id == id; }),
                        appointments_.end());
                    waiting_.erase(std::remove_if(waiting_.begin(), waiting_.end(),
                        [&](const QueueEntry& entry) { return entry.patient_id == id; }), waiting_.end());
                });
                std::cout << "[OK] Patient deleted.\n";
                break;
            }
            }
        }
    }

    void appointments_menu() {
        while (true) {
            std::cout << "\n-- Appointments --\n1 Schedule  2 List  3 Search by patient  4 Cancel  0 Back\n";
            switch (number("Choice: ", 0, 4)) {
            case 0: return;
            case 1: {
                std::string patient_id = prompt("Patient ID: ");
                if (!find_patient(patient_id)) {
                    std::cout << "[ERROR] Patient does not exist.\n";
                    break;
                }
                for (std::size_t i = 0; i < doctors_.size(); ++i)
                    std::cout << i + 1 << ") " << doctors_[i] << '\n';
                int doctor_choice = number("Doctor: ", 1, static_cast<int>(doctors_.size()));
                std::string date = prompt("Date (YYYY-MM-DD): ");
                std::string time = prompt("Time (HH:MM): ");
                if (!valid_date(date) || !valid_time(time)) {
                    std::cout << "[ERROR] Invalid date or time.\n";
                    break;
                }
                const std::string& doctor = doctors_[doctor_choice - 1];
                bool conflict = std::any_of(appointments_.begin(), appointments_.end(),
                    [&](const Appointment& appointment) {
                        return !appointment.cancelled && appointment.doctor == doctor &&
                               appointment.date == date && appointment.time == time;
                    });
                if (conflict) {
                    std::cout << "[ERROR] Doctor already has an appointment at that time.\n";
                    break;
                }
                Appointment appointment{make_id('A', next_appointment_id_),
                                        patient_id, doctor, date, time, false};
                save_change([&] {
                    appointments_.push_back(std::move(appointment));
                    ++next_appointment_id_;
                });
                std::cout << "[OK] Appointment scheduled.\n";
                break;
            }
            case 2: {
                auto appointments = appointments_;
                insertion_sort(appointments);
                if (appointments.empty()) std::cout << "No appointments.\n";
                for (const auto& appointment : appointments)
                    std::cout << appointment.id << " | " << appointment.date << ' '
                              << appointment.time << " | " << appointment.patient_id << " | "
                              << appointment.doctor << " | "
                              << (appointment.cancelled ? "Cancelled" : "Scheduled") << '\n';
                break;
            }
            case 3: {
                std::string patient_id = prompt("Patient ID: ");
                bool found = false;
                for (const auto& appointment : appointments_) {
                    if (appointment.patient_id == patient_id) {
                        std::cout << appointment.id << " | " << appointment.date << ' '
                                  << appointment.time << " | " << appointment.doctor << " | "
                                  << (appointment.cancelled ? "Cancelled" : "Scheduled") << '\n';
                        found = true;
                    }
                }
                if (!found) std::cout << "No appointments found.\n";
                break;
            }
            case 4: {
                std::string id = prompt("Appointment ID to cancel: ");
                auto found = std::find_if(appointments_.begin(), appointments_.end(),
                    [&](const Appointment& appointment) {
                        return appointment.id == id && !appointment.cancelled;
                    });
                if (found == appointments_.end()) {
                    std::cout << "[WARN] Appointment not found.\n";
                    break;
                }
                save_change([&] { found->cancelled = true; });
                std::cout << "[OK] Appointment cancelled.\n";
                break;
            }
            }
        }
    }

    void waiting_menu() {
        while (true) {
            std::cout << "\n-- Waiting Queue --\n1 Add patient  2 Call next  3 Display  0 Back\n";
            switch (number("Choice: ", 0, 3)) {
            case 0: return;
            case 1: {
                std::string id = prompt("Patient ID: ");
                if (!find_patient(id)) {
                    std::cout << "[ERROR] Patient not found.\n";
                    break;
                }
                if (std::any_of(waiting_.begin(), waiting_.end(),
                    [&](const QueueEntry& entry) { return entry.patient_id == id; })) {
                    std::cout << "[WARN] Patient is already waiting.\n";
                    break;
                }
                std::cout << "Priority 1=Emergency (illustrative), 2=Normal; not a clinical protocol.\n";
                int priority = number("Priority: ", 1, 2);
                waiting_.push_back({priority, arrival_order_++, id});
                std::cout << "[OK] Added to queue.\n";
                break;
            }
            case 2: {
                StablePriorityQueue queue;
                for (const auto& entry : waiting_) queue.push(entry);
                QueueEntry next{};
                if (!queue.pop(next)) {
                    std::cout << "[WARN] Queue is empty.\n";
                    break;
                }
                Patient* patient = find_patient(next.patient_id);
                std::cout << "[OK] Call " << (patient ? patient->name : next.patient_id) << ".\n";
                waiting_.erase(std::remove_if(waiting_.begin(), waiting_.end(),
                    [&](const QueueEntry& entry) { return entry.patient_id == next.patient_id; }),
                    waiting_.end());
                break;
            }
            case 3: {
                auto entries = waiting_;
                std::stable_sort(entries.begin(), entries.end(),
                    [](const QueueEntry& a, const QueueEntry& b) {
                        return a.priority < b.priority;
                    });
                for (const auto& entry : entries)
                    std::cout << entry.patient_id << " | "
                              << (entry.priority == 1 ? "Emergency (illustrative)" : "Normal") << '\n';
                if (entries.empty()) std::cout << "Queue is empty.\n";
                break;
            }
            }
        }
    }

    void history_menu() {
        Patient* patient = find_patient(prompt("Patient ID: "));
        if (!patient) {
            std::cout << "[WARN] Patient not found.\n";
            return;
        }
        std::cout << "1 Add visit  2 View visits  0 Back\n";
        int choice = number("Choice: ", 0, 2);
        if (choice == 1) {
            Visit visit;
            visit.date = prompt("Date (YYYY-MM-DD): ");
            if (!valid_date(visit.date)) {
                std::cout << "[ERROR] Invalid date.\n";
                return;
            }
            visit.department = prompt("Department: ");
            visit.note = prompt("Brief administrative note: ");
            save_change([&] { patient->visits.push_back(std::move(visit)); });
            std::cout << "[OK] Visit recorded.\n";
        } else if (choice == 2) {
            auto visits = patient->visits.values();
            insertion_sort(visits);
            for (const auto& visit : visits)
                std::cout << visit.date << " | " << visit.department << " | " << visit.note << '\n';
            if (visits.empty()) std::cout << "No visits recorded.\n";
        }
    }

    void reports() const {
        std::cout << "\nPatients: " << patients_.size() << "\nWaiting: " << waiting_.size() << '\n';
        std::size_t scheduled = std::count_if(appointments_.begin(), appointments_.end(),
            [](const Appointment& appointment) { return !appointment.cancelled; });
        std::cout << "Scheduled appointments: " << scheduled << '\n';
    }

public:
    Hospital() { load(); }

    void run() {
        while (true) {
            std::cout << "\n=== Hospital Patient Management ===\n"
                      << "1 Patients  2 Appointments  3 Waiting Queue\n"
                      << "4 Visit History  5 Reports  0 Exit\n";
            try {
                switch (number("Choice: ", 0, 5)) {
                case 0: return;
                case 1: patients_menu(); break;
                case 2: appointments_menu(); break;
                case 3: waiting_menu(); break;
                case 4: history_menu(); break;
                case 5: reports(); break;
                }
            } catch (const std::exception& error) {
                if (std::cin.eof()) return;
                std::cout << "[ERROR] " << error.what() << '\n';
            }
        }
    }
};

#ifndef HOSPITAL_DSA_TESTING
int main() {
    try {
        Hospital hospital;
        hospital.run();
    } catch (const std::exception& error) {
        std::cerr << "[FATAL] " << error.what() << '\n';
        return 1;
    }
}
#endif
