<!-- group-68 -->
# AcadERP — Academic Records & Graduation Audit System

**Group 68**
- Jalakam Chandra Harsha (MT25022)
- Shivam Minde (MT25043)

## Build Steps

```bash
# Build
make

# Run
./acaderp < input.txt > output.txt

# Run tests
chmod +x run_tests.sh
./run_tests.sh

# Clean
make clean
```

## Design Summary

### Architecture
The system follows a layered OOP architecture:

1. **Foundation Layer**: `Grade` (immutable value object with factory pattern), `ProgramRules` (configuration), `CourseMaterial` (abstract base with deep-copy via `clone()`)

2. **Course Hierarchy**: Abstract `Course` base class with three concrete types:
   - `RegularCourse`: Accepts letter grades (A through F) and I
   - `SuCourse`: Accepts S, X, I
   - `ThesisCourse`: Accepts S, X (progress grades only)

   Each overrides `validateGrade()`, `countsInGpa()`, and `earnsCredits()`.

3. **Domain Layer**:
   - `ThesisRecord`: Encapsulates the thesis state machine (REGISTERED → IN_PROGRESS → FINAL, with AT_RISK and TERMINATED paths)
   - `Attempt`: One registration of a student in a course/term, with grade history
   - `Student`: Holds all attempts, provides counting-attempt lookup

4. **Application Layer**:
   - `Registrar`: Facade class implementing all business logic (registration, grading, GPA, graduation, eligibility)
   - `CommandParser`: Parses input commands and dispatches to Registrar

### Exception Hierarchy
All exceptions derive from `ErpException` (which derives from `std::runtime_error`). Intermediate categories: `InputException`, `EntityException`, `GradeException`, `RegistrationException`, `ComputationException`. Each leaf exception carries a fixed error code (e.g., `E_PARSE`, `E_GRADE_TYPE`).

### GPA Calculation
Uses **integer arithmetic only** (no `float`/`double`). The GPA is computed as:
- `numerator = Σ(credits × grade_points)` for counting attempts of regular courses and FINAL theses
- `denominator = Σ(credits)` for those same attempts
- `hundredths = ⌊(200 × numerator + denominator) / (2 × denominator)⌋`
- Printed as `X.YY` with zero-padding

This avoids the `printf("%.2f", 8.125) → 8.12` bug that floating-point rounding causes.

### CourseMaterial Deep Copy
`CourseMaterial` is an abstract base with a virtual `clone()` returning `std::unique_ptr<CourseMaterial>`. Concrete types (`Syllabus`, `LectureNotes`, `Assignment`) implement deep copy, allowing instructors to copy and modify course materials independently.

### Thesis State Machine
States: REGISTERED, IN_PROGRESS, AT_RISK, TERMINATED, FINAL.
- Only IN_PROGRESS can be CONVERT-ed to a letter grade
- Two consecutive X progress grades → TERMINATED
- TERMINATED and FINAL are terminal states
- Progress grades must be for terms in strictly increasing order

## Assumptions

1. **Term format**: Terms are in `YYYY-S` format where `S ∈ {1, 2, 3}`. The semester number represents semesters within a year.

2. **Prerequisite default grade**: Default minimum grade for prerequisites is D (4 points). For SU prerequisites, the check is for grade S regardless of any specified minimum.

3. **Grade improvement tracking**: A student has "used" their one improvement for a course if they have more than one passing attempt for that course.

4. **ELIGIBLE vs REGISTER**: ELIGIBLE throws for E1 (unknown student/course) and reports E2-E7 as codes. REGISTER throws for all E1-E7.

5. **Thesis re-registration**: After a TERMINATED thesis or a FINAL thesis with a failing grade (E/F), the student may register for the same thesis course again, subject to E7 checks.

6. **Credit load**: A thesis course counts toward the credit load only in its start (registration) term, not in subsequent terms when progress grades are submitted.

7. **CONVERT grade validation**: The letter grade in CONVERT must be a valid letter grade (A through F). S, X, I are not valid for CONVERT.

8. **E7 thesis check**: "No other thesis open or passed" means no thesis attempt (any thesis course) is currently in REGISTERED/IN_PROGRESS/AT_RISK state, and no thesis attempt has a FINAL state with a passing grade.

9. **I grade replacement**: An I grade can be replaced exactly once with a final grade of the same course type. Giving I again after I is an error.

10. **Blank lines and comments**: Blank lines and lines starting with `#` are skipped but counted for line numbering.

11. **PROGRAM command**: Optional; defaults apply if not specified. Must come before any COURSE command.

12. **TRANSCRIPT format**: Displays student roll, name, and a list of all attempts with course code, term, grade(s), and thesis state (if applicable).

13. **R5 thesis check**: At least one thesis must be in FINAL state with a converted letter grade ≥ `thesis_min_grade`.

14. **Strong exception guarantee**: All commands that modify state validate all preconditions before making any changes. If validation fails (exception thrown), no state is modified.

15. **Counting attempt**: The counting attempt for a course is the latest attempt that has a final grade. For thesis courses, a final grade means the thesis is in FINAL state with a converted letter.

## Performance

### Scalability Discussion (3000+ students)

**Current Data Structures**:
- Students and courses stored in `std::map<string, unique_ptr<...>>` for O(log n) lookup.
- Attempts stored in vectors per student.

**Bottlenecks and Solutions for 3000+ students**:

1. **Student/Course Lookup**: Current O(log n) with `std::map`. Could be upgraded to `std::unordered_map` for O(1) average lookup. With 3000 students, log(3000) ≈ 12, so maps are still fast.

2. **GPA Computation**: O(k) per student where k = number of courses taken. Consider caching GPA and invalidating on grade changes for amortized O(1).

3. **Graduation Checks**: Iterates all courses for R2/R4 checks. With a course catalog of ~100 courses, this is O(100) per check — acceptable. Could index courses by category for faster filtering.

4. **Eligibility Checks**: E4 (prerequisites) and E5 (anti-requisites) are bounded by prerequisite count, typically O(1) to O(5). E6 (credit load) scans student attempts — could be cached per term.

5. **Memory**: Each student has ~10-30 attempts on average. With 3000 students, ~30,000-90,000 Attempt objects. At ~100 bytes each, this is ~3-9 MB — negligible.

6. **Batch Processing**: Within an hour, the system processes commands sequentially. The bottleneck is I/O, not computation. Using `std::ios_base::sync_with_stdio(false)` and `std::cin.tie(nullptr)` would improve I/O performance.

7. **Custom Allocator**: For high-frequency allocation patterns (many Attempt and Student objects), a pool allocator reduces allocation overhead and eliminates memory fragmentation.

**Conclusion**: The current design handles 3000+ students comfortably within an hour. For 100,000+ students, replacing `std::map` with `std::unordered_map` and adding GPA caching would be the primary optimizations.

## Bonus Features (+10 Marks)

### 1. State Serialization: `SAVE <file>` & `LOAD <file>` (+4 Marks)
- **Syntax**: `SAVE <filepath>` and `LOAD <filepath>`.
- **Implementation**: The system serializes its entire runtime state (active program rules, registered courses, prerequisite dependency graphs, anti-requisite constraints, registered students, and full academic history with thesis lifecycle progression) into a canonical command script.
- **Round-trip Guarantee**: `LOAD` resets the Registrar state and reconstructs the domain model identically, demonstrated in automated test `T13`.

### 2. Grade Change Workflow with Security Token & Audit Logging (+3 Marks)
- **Syntax**: `CHANGE <token> <roll> <course> <term> <new_symbol> <reason...>`
- **Authorization**: Grade modifications on finalized attempts are strictly restricted to the `DEAN` token. Unauthorized tokens raise typed `UnauthorizedException` (`E_UNAUTHORIZED`).
- **Audit Log**: Every authorized grade change generates an immutable log record capturing timestamp, student roll, course code, term, previous grade, updated grade, and the administrative justification. Entries are retained in-memory and persistently appended to `audit.log`.

### 3. Custom Memory Pool Allocator: `StudentPoolAllocator` (+3 Marks)
- **Header**: `src/grp_68_StudentAllocator.h`
- **Architecture**:
  - **Slab Allocation**: Pre-allocates memory in large contiguous slabs (blocks of 1,024 elements), amortizing system `malloc`/`new` overhead to $O(1)$ per batch.
  - **Embedded Free-List**: Deallocated student/attempt records are recycled in $O(1)$ time via an embedded singly-linked free list, completely eliminating heap fragmentation during registration churn.
  - **Dynamic Compaction**: Live element counts are tracked per slab. When large cohorts graduate and depart simultaneously, empty slabs can be relinquished back to the operating system, gracefully handling large-scale population decrease.
  - **Cache Locality**: Contiguous object layout maximizes L1/L2 data cache hit rates when computing bulk GPAs and graduation audits across 3,000+ students.
  - **Standard Compliance**: Conforms to C++17 `std::allocator_traits` for seamless integration with STL containers (`std::vector`, `std::list`, etc.).


