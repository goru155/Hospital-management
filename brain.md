# Design rationale

## Goal and scope

This college project models basic reception tasks: patient registration and lookup, appointments, waiting order, visit history, and simple totals. It uses only a few DSA structures where they fit naturally in those workflows. It is educational software, not a clinical system.

## Why these structures are integrated

- Patients and appointments are kept in vectors because a small flat-file application benefits from simple iteration, display, and serialization.
- A custom **BST patient-ID index** makes exact ID lookups a real application operation instead of a standalone tree demo. The index stores vector positions, is built on load, updated on registration, and rebuilt after deletion because erasing a vector element shifts later positions. The tree is unbalanced, so worst-case lookup is O(n).
- A **singly linked list** holds each patient's visit history, demonstrating dynamically allocated nodes in a meaningful record feature. It supports append and traversal and has value-copy behavior so persistence rollback does not share or leak nodes.
- A **stable binary-heap priority queue** orders waiting patients: a smaller numeric priority comes first and equal priorities are processed FIFO. The queue state lasts for one run. Priority labels are illustrative and are not a clinical triage protocol.
- **Insertion sort** orders the small appointment and history views chronologically. Its O(n²) cost is acceptable for a teaching-sized dataset.

Other syllabus structures (stacks, hash tables, graph algorithms, AVL/B-trees, matrix multiplication, and alternate linked/queue types) are not forced into unrelated hospital behavior and are intentionally omitted from the app. Their coverage status is listed in `syllabus-mapping.md`.

## Language and organization

C++17 supports the course's focus on structures and pointers and provides portable file streams and standard containers without third-party dependencies. A terminal interface makes the operations easy to demonstrate.

All application code is in `src/main.cpp`; focused tests live in `tests/test_dsa.cpp`. Keeping a single source file avoids separate headers and include paths, at the cost of a longer file. The DSA namespace holds reusable structures; one `using namespace dsa;` makes them available to the application and included tests.

## Persistence and correctness

Patient IDs and appointment IDs use persisted sequences. The patient-index BST is reconstructed from patient records on load rather than serialized separately, avoiding two competing sources of truth. Deleting a patient also removes associated appointments and waiting entries.

Patients, linked-list visit histories, appointments, and ID counters are saved in `data/hospital.db`. Saves use a temporary file and retain the previous version as `.bak`. Loading parses into temporary record collections before activation. Waiting entries are session-only.

## Complexity and trade-offs

For n patients, vector iteration and deletion are O(n). A balanced BST would provide O(log n) search, but this plain BST is O(h) and can degrade to O(n); this makes the syllabus structure visible and its trade-off easy to discuss. Linked-list append is O(1), history traversal O(n), priority-queue push/pop O(log n), and insertion sort O(n²). Big O is an asymptotic upper bound, Big Omega a lower bound, and Big Theta a tight bound.

## Limitations

There is no authentication, encryption, concurrent access, database transaction system, or clinical validation. Appointments use simple date/time strings and waiting entries are not persistent. Use invented data only.
