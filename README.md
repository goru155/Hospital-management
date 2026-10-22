# Hospital Patient Management System

A small C++17 command-line project for patient records, appointments, waiting order, and visit history. The selected DSA topics are part of those workflows: a BST indexes patient IDs, a singly linked list stores visit history, a stable heap-based priority queue manages the waiting line, and insertion sort orders appointments.

> Educational project only. It is not clinically validated; use fictional data only.

## Build and run

```text
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp -o hospital
./hospital
```

On Windows, use `-o hospital.exe` and run `.\hospital.exe`. Build and run focused DSA tests with:

```text
g++ -std=c++17 -Wall -Wextra -pedantic tests/test_dsa.cpp -o dsa_tests
./dsa_tests
```

## Files

- `src/main.cpp` — application and DSA implementations used by its workflows.
- `tests/test_dsa.cpp` — assertions for the integrated structures.
- `syllabus-mapping.md` — selected syllabus coverage and omissions.
- `documentation.md` — menu, build, persistence, and complexity notes.

Records are saved under `data/hospital.db`. See [documentation.md](./documentation.md) and [syllabus-mapping.md](./syllabus-mapping.md).
