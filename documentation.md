# Project guide

## Purpose

This C++17 terminal program manages a small set of fictional patient records, appointments, consultation waiting entries, and administrative visit notes. Data structures are used in those workflows rather than shown in a separate DSA demonstration menu:

- A **binary search tree (BST)** indexes patient IDs for patient lookups.
- A **singly linked list** stores each patient's visit history.
- A **stable binary-heap priority queue** chooses the next waiting patient; lower priority number is served first, and arrival order breaks ties.
- **Insertion sort** displays appointments and visits chronologically.
- `std::vector` stores the main patient and appointment collections and is used for serialization.

The emergency/normal priority labels are illustrative only, not a clinical triage protocol. This is a college exercise, not a validated medical system. Do not enter real patient details.

## Layout

```text
.
|-- src/
|   `-- main.cpp
|-- tests/
|   `-- test_dsa.cpp
|-- data/                 created after the first saved record
|-- README.md
|-- documentation.md
|-- brain.md
|-- future-scope.md
|-- syllabus-mapping.md
|-- student-tasks.md
`-- Makefile
```

The source file contains the models, custom structures, application menus, persistence, and `main`. There are no header files or separate DSA showcase menu. The source uses `using namespace dsa;` once after the DSA definitions; standard-library names stay qualified with `std::`.

## Build and run

Windows with g++:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -o hospital.exe
.\hospital.exe
```

Linux:

```sh
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -o hospital
./hospital
```

Tests:

```text
g++ -std=c++17 -Wall -Wextra -pedantic tests/test_dsa.cpp -o dsa_tests
./dsa_tests
```

Run the program from the project directory because its data path is relative. GNU Make is optional: `make` builds the program and `make test` runs the tests.

## Menus and workflows

1. **Patients** — register, search by exact ID or name substring, list, update basic details, and delete with confirmation. Deletion also removes the patient's appointments and waiting entry.
2. **Appointments** — schedule for an existing patient, reject a conflicting doctor/date/time slot, list chronologically, search by patient ID, and cancel.
3. **Waiting Queue** — add an existing patient with priority 1 or 2, display waiting order, or call next.
4. **Visit History** — add a dated administrative note or view the patient's notes chronologically.
5. **Reports** — show patient, waiting, and scheduled-appointment counts.
0. **Exit**.

Dates use `YYYY-MM-DD` and 24-hour time uses `HH:MM`. Menu and age inputs are validated. Patient and appointment IDs have incrementing sequences which are saved in the data file.

## Data structures and complexity

| Structure / algorithm | Use | Complexity |
|---|---|---|
| `PatientIndex` (BST) | Exact patient-ID lookup, rebuilt from records after loading and after deletion | O(h) search/insert; worst case O(n) |
| `SinglyList<Visit>` | Per-patient visit history; copyable so failed file saves can restore prior state | O(n) traversal/copy; O(1) append |
| `StablePriorityQueue` | Live waiting list; stable order for ties | O(log n) push/pop |
| Insertion sort | Chronological appointment and visit display | O(n²) time; O(1) auxiliary space |
| `std::vector` | Persistent patient and appointment records | O(n) search and deletion |

Big O is an asymptotic upper bound, Big Omega a lower bound, and Big Theta a tight bound. The BST is not self-balancing, so sorted IDs can make it linear. Keeping this application small makes that trade-off visible without introducing AVL-tree complexity.

## Persistence and error handling

`data/hospital.db` stores a version tag, next-ID counters, patients with their linked-list visits, and appointments. Saves go through a temporary file and retain the previous file as `data/hospital.db.bak`. Loading parses into temporary vectors before replacing current records; invalid input causes an error rather than silently starting with empty records. Waiting entries are session-only.

This flat-file design is not a database. It has no multi-user coordination, encryption, authentication, or full transaction guarantees.

## Tests and omissions

`tests/test_dsa.cpp` checks linked-list copy/traversal, patient-index lookup, stable priority order, and chronological sorting. Run the application manually to verify patient registration, appointments, waiting order, visit history, and restart persistence.

The reduced project intentionally omits standalone demonstrations of stacks, hash tables, graph algorithms, matrix multiplication, AVL/B-trees, circular/doubly linked lists, and other syllabus material. This is a selected project scope, not a claim that the complete course syllabus has been implemented; see [syllabus-mapping.md](./syllabus-mapping.md).
