/*
 * =====================================================================================
 *  GROUP 16 — UNIVERSITY LIBRARY BORROWING, FINES AND BOOK-STATE TRACKER
 *  Concepts demonstrated: Aliases | Pointers/References | Explicit heap-dynamic
 *                         storage | Lifetime | Scope vs Lifetime
 *
 *  Compile:  g++ -std=c++17 -Wall -o library_system library_system.cpp
 *  Run:      ./library_system
 * =====================================================================================
 */

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

using namespace std;

// -------------------------------------------------------------------------------------
// A. NAMED CONSTANTS  (bound once, at compile time — never change during execution)
// -------------------------------------------------------------------------------------
const int    MAX_LOAN_DAYS   = 14;   // loan period
const double CHARGE_PER_DAY  = 20.0; // training overdue charge (fictional, per lab note)

// -------------------------------------------------------------------------------------
// A. DATA MODEL
// -------------------------------------------------------------------------------------
struct Book {
    int    accessionNo;
    string title;
    int    availableCopies;
    string status;   // "Available" / "Unavailable"
};

struct Transaction {
    int    studentId;
    string studentName;
    int    accessionNo;
    int    borrowDay;
    int    dueDay;
    int    returnDay;   // -1 => still out on loan
    double fine;
};

// Global (program) scope, program (static) lifetime — exists for the whole run.
vector<Book>        library;
vector<Transaction> transactionLog;
int totalSuccessfulBorrows = 0;
int totalFailedBorrows     = 0;

// -------------------------------------------------------------------------------------
// A. WORKFLOW FUNCTIONS
// -------------------------------------------------------------------------------------

// Ordinary function-local variable "searchIndex": stack-dynamic storage.
// Its lifetime BEGINS when searchBook() is entered and ENDS the instant it returns.
// (Contrast this with dynamicBook in main(), which outlives many function calls.)
int searchBook(const vector<Book>& lib, int accessionNo) {
    int searchIndex = 0;                       // <-- lifetime starts here
    while (searchIndex < (int)lib.size()) {
        if (lib[searchIndex].accessionNo == accessionNo)
            return searchIndex;
        searchIndex++;
    }
    return -1;
}                                               // <-- searchIndex's storage dies here

bool borrowBook(vector<Book>& lib, vector<Transaction>& log,
                int studentId, const string& studentName,
                int accessionNo, int today) {

    // D. SCOPE vs LIFETIME:
    // totalLoans is a STATIC LOCAL variable.
    //   Scope    -> local to borrowBook() only; invisible anywhere else in the program.
    //   Lifetime -> spans the ENTIRE program run; it is initialised once and keeps its
    //               value across every future call to borrowBook().
    static int totalLoans = 0;

    int idx = searchBook(lib, accessionNo);
    if (idx == -1) {
        cout << "  [FAIL] No book with accession number " << accessionNo << ".\n";
        totalFailedBorrows++;
        return false;
    }
    if (lib[idx].availableCopies <= 0) {
        cout << "  [FAIL] \"" << lib[idx].title << "\" has no available copies.\n";
        totalFailedBorrows++;
        return false;
    }

    lib[idx].availableCopies--;
    if (lib[idx].availableCopies == 0) lib[idx].status = "Unavailable";

    Transaction t{studentId, studentName, accessionNo, today, today + MAX_LOAN_DAYS, -1, 0.0};
    log.push_back(t);

    totalLoans++;
    totalSuccessfulBorrows++;
    cout << "  [OK]   Loan #" << totalLoans << " (static lifetime counter) -> "
         << studentName << " borrowed \"" << lib[idx].title << "\" (due day " << t.dueDay << ")\n";
    return true;
}

double calculateFine(int dueDay, int returnDay) {
    if (returnDay <= dueDay) return 0.0;
    int daysLate = returnDay - dueDay;
    return daysLate * CHARGE_PER_DAY;           // CHARGE_PER_DAY resolved from global scope
}

bool returnBook(vector<Book>& lib, vector<Transaction>& log,
                int accessionNo, int studentId, int today) {
    for (auto& t : log) {
        if (t.accessionNo == accessionNo && t.studentId == studentId && t.returnDay == -1) {
            t.returnDay = today;
            t.fine      = calculateFine(t.dueDay, today);

            int idx = searchBook(lib, accessionNo);
            if (idx != -1) {
                lib[idx].availableCopies++;
                lib[idx].status = "Available";
            }
            cout << "  [OK]   " << t.studentName << " returned accession " << accessionNo;
            if (t.fine > 0)
                cout << " -- LATE, fine = KES " << fixed << setprecision(2) << t.fine;
            cout << "\n";
            return true;
        }
    }
    cout << "  [FAIL] No open loan found for student " << studentId
         << " on accession " << accessionNo << ".\n";
    totalFailedBorrows++;
    return false;
}

// -------------------------------------------------------------------------------------
// B. ALIASING EXPERIMENT
// Two different NAMES bound to the SAME storage location. Changing the object through
// one name is visible through the other, because there is only one object, not two.
// -------------------------------------------------------------------------------------
void demonstrateAliasing(vector<Book>& lib) {
    cout << "\n--- B. Aliasing experiment (Book& references) ---\n";
    if (lib.empty()) return;

    Book& ref1 = lib[0];   // ref1 is an alias for lib[0]
    Book& ref2 = ref1;     // ref2 is a second alias for the very same object as ref1

    cout << "Before: ref1.availableCopies=" << ref1.availableCopies
         << "  ref2.availableCopies=" << ref2.availableCopies << "  (same object)\n";

    ref1.availableCopies -= 1;   // modification made ONLY through ref1

    cout << "After modifying ref1 only: ref1.availableCopies=" << ref1.availableCopies
         << "  ref2.availableCopies=" << ref2.availableCopies
         << "  <-- ref2 changed too: proof they share one storage location\n";

    ref1.availableCopies += 1;   // restore for the rest of the simulation
}

// -------------------------------------------------------------------------------------
// C. LIFETIME / EXPLICIT HEAP-DYNAMIC STORAGE EXPERIMENT
// -------------------------------------------------------------------------------------
void inspectBook(const Book* b, const string& label) {
    cout << "  [" << label << "] accession=" << b->accessionNo
         << " title=\"" << b->title << "\" copies=" << b->availableCopies
         << " status=" << b->status << "\n";
}

// -------------------------------------------------------------------------------------
// OUTPUT / REPORT FUNCTIONS
// -------------------------------------------------------------------------------------
void printStockTable(const vector<Book>& lib) {
    cout << "\n=== CURRENT STOCK TABLE ===\n";
    cout << left << setw(10) << "Acc.No" << setw(38) << "Title"
         << setw(10) << "Copies" << "Status\n";
    for (const auto& b : lib) {
        cout << left << setw(10) << b.accessionNo << setw(38) << b.title
             << setw(10) << b.availableCopies << b.status << "\n";
    }
}

void printTransactionLog(const vector<Transaction>& log) {
    cout << "\n=== TRANSACTION LOG ===\n";
    cout << left << setw(6) << "StuID" << setw(16) << "Name" << setw(8) << "Acc.No"
         << setw(10) << "Borrow" << setw(6) << "Due" << setw(10) << "Return"
         << "Fine(KES)\n";
    for (const auto& t : log) {
        cout << left << setw(6) << t.studentId << setw(16) << t.studentName
             << setw(8) << t.accessionNo << setw(10) << t.borrowDay
             << setw(6) << t.dueDay
             << setw(10) << (t.returnDay == -1 ? string("-- out --") : to_string(t.returnDay))
             << fixed << setprecision(2) << t.fine << "\n";
    }
}

void printOverdueCharges(const vector<Transaction>& log) {
    cout << "\n=== OVERDUE CHARGES ===\n";
    bool any = false;
    for (const auto& t : log) {
        if (t.fine > 0) {
            cout << "  " << t.studentName << " (accession " << t.accessionNo
                 << ") owes KES " << fixed << setprecision(2) << t.fine << "\n";
            any = true;
        }
    }
    if (!any) cout << "  No overdue charges.\n";
}

// =======================================================================================
int main() {
    cout << fixed << setprecision(2);

    // ---- Seed at least 8 books ----
    library = {
        {1001, "Introduction to Algorithms",              2, "Available"},
        {1002, "Clean Code",                               1, "Available"},
        {1003, "The C Programming Language",               3, "Available"},
        {1004, "Database System Concepts",                 1, "Available"},
        {1005, "Computer Networking: A Top-Down Approach",  2, "Available"},
        {1006, "Operating System Concepts",                 1, "Available"},
        {1007, "Discrete Mathematics and Its Applications", 2, "Available"},
        {1008, "Software Engineering: A Practitioner's Approach", 1, "Available"},
    };

    cout << "======================================================\n";
    cout << " A. INITIAL STOCK\n";
    cout << "======================================================\n";
    printStockTable(library);

    // ---- B. Aliasing experiment ----
    demonstrateAliasing(library);

    // ---- C. Explicit heap-dynamic storage & lifetime experiment ----
    cout << "\n--- C. Explicit heap-dynamic storage & lifetime experiment ---\n";
    // dynamicBook is created on the HEAP. Its lifetime is controlled explicitly by the
    // programmer (new ... delete), NOT tied to any single function's scope.
    Book* dynamicBook = new Book{9009, "Design Patterns (Gang of Four)", 2, "Available"};

    inspectBook(dynamicBook, "call 1, right after creation");
    library.push_back(*dynamicBook);           // also list it in the catalogue for borrowing

    dynamicBook->availableCopies--;            // simulate an external change to the same data
    inspectBook(dynamicBook, "call 2, after a copy was taken elsewhere");

    cout << "  NOTE: dynamicBook has now survived TWO separate calls to inspectBook().\n"
         << "        Compare this with searchIndex inside searchBook(): that local variable\n"
         << "        is destroyed the instant searchBook() returns, on every single call.\n"
         << "        dynamicBook's heap storage will only end when we explicitly delete it.\n";

    // ---- A/B/C/D. Borrowing and returning (>=5 transactions) ----
    cout << "\n======================================================\n";
    cout << " A. BORROW / RETURN WORKFLOW (day-numbered simulation)\n";
    cout << "======================================================\n";

    borrowBook(library, transactionLog, 2026001, "Amina Cherono", 1001, 1);
    borrowBook(library, transactionLog, 2026002, "Brian Otieno",  1002, 1);
    borrowBook(library, transactionLog, 2026003, "Faith Wanjiru", 1004, 2);
    borrowBook(library, transactionLog, 2026004, "Kevin Mutai",   1006, 2);
    borrowBook(library, transactionLog, 2026005, "Diana Chebet",  9009, 3);   // borrows the (copied-in) dynamicBook entry
    borrowBook(library, transactionLog, 2026006, "Amina Cherono", 1004, 3);   // exceptional: no copies left -> should fail
    borrowBook(library, transactionLog, 2026007, "Brian Otieno",  9999, 4);   // exceptional: unknown accession -> fail

    cout << "\n-- Returns --\n";
    returnBook(library, transactionLog, 1001, 2026001, 10);   // on time
    returnBook(library, transactionLog, 1002, 2026002, 20);   // late -> fine
    returnBook(library, transactionLog, 1004, 2026003, 12);   // on time
    returnBook(library, transactionLog, 1006, 2026099, 15);   // exceptional: wrong student id -> fail

    // ---- D. Scope vs lifetime: additional example ----
    cout << "\n--- D. Scope vs lifetime: additional example ---\n";
    cout << "  The Book objects inside the global vector 'library' stay alive for as long\n"
         << "  as the program runs (their lifetime = the whole program), yet the local\n"
         << "  variable 'idx' used inside searchBook()/borrowBook() to reach a given Book\n"
         << "  goes out of scope the moment those functions return. The object outlives\n"
         << "  the name that was used to reach it -- an object can be ALIVE even when it\n"
         << "  is not currently VISIBLE through any particular local name.\n";

    // Release the dynamically allocated object at the correct point, now that we are done
    // with it as a standalone pointer (its data already lives on inside the 'library' vector).
    delete dynamicBook;
    dynamicBook = nullptr;
    cout << "\n  dynamicBook has now been explicitly deleted -- its heap lifetime has ended.\n";

    // ---- E. Reports ----
    cout << "\n======================================================\n";
    cout << " E. FINAL REPORTS\n";
    cout << "======================================================\n";
    printTransactionLog(transactionLog);
    printStockTable(library);
    printOverdueCharges(transactionLog);

    cout << "\n=== SUMMARY ===\n";
    cout << "Total successful borrowing attempts: " << totalSuccessfulBorrows << "\n";
    cout << "Total failed borrowing attempts:     " << totalFailedBorrows << "\n";

    return 0;
}
