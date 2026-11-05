# SmartJatri

A console-based bus card system written in C++. Passengers tap a card by UID to pay their fare. Admins issue cards, recharge balances, and review trip history.

Originally written in C for CSE 207 (Data Structures) in May 2025. This repository is the C++ rewrite.

## Features

- Four card types: General, Student, Tourist, and Child, each with its own fare and validity period
- Admin panel to add, search, list, and delete cards
- Balance recharge and low-balance warnings
- Expiry checks, with a notice when a card is about to expire
- Trip history: the last 10 trips per card
- Data saved to disk and reloaded on startup
- Admin login with a salted password hash

## Fares and validity

| Card type | Fare (Tk) | Valid for | UID prefix |
|-----------|-----------|-----------|------------|
| General   | 100.00    | 5 years   | `G`        |
| Student   | 50.00     | 2 years   | `S`        |
| Tourist   | 130.00    | 1 month   | `T`        |
| Child     | 50.00     | 5 years   | `C`        |

UIDs are generated automatically, for example `S0001`.

## Requirements

- A C++14 or later compiler, such as GCC (MinGW-w64) 7 or newer
- Code::Blocks (optional, for the IDE build)
- Windows is recommended. Password masking uses `conio.h`, so on other systems the password is visible as you type.

## Building

### Code::Blocks

1. Open the project file `SmartJatri.cbp`.
2. Press **Ctrl+F9** to build and **Ctrl+F10** to run.

The project already sets `-std=c++17`.

### Command line

```bash
g++ -std=c++17 -Wall -o smartjatri main.cpp AdminAuth.cpp Date.cpp Kiosk.cpp User.cpp UserRepository.cpp Utils.cpp
./smartjatri
```

On Windows, run `smartjatri.exe`.

## Usage

On first launch, you'll be asked to create an admin account. The main menu then offers:

```text
=====================================
   Welcome to Digital Bus Card System
=====================================
  1. Tap card (passenger)
  2. Admin login
  0. Exit
```

### Passenger

1. Choose **1** and enter your card UID. UIDs aren't case-sensitive.
2. Enter the boarding and destination stations.
3. The fare is deducted and the trip is recorded.

### Admin

After logging in, the admin panel offers:

1. **Add user**: issue a new card
2. **Delete user**: remove a card (type `YES` to confirm)
3. **Search user**: show a card's details
4. **List all users**: show every card with its balance and expiry date
5. **Recharge card**: add money to a card
6. **View trip history**: show the last 10 trips for a card

## Data files

The program creates these files in its working directory:

| File        | Contents                                    |
|-------------|---------------------------------------------|
| `users.txt` | Card records and trip history               |
| `admin.txt` | Admin username, salt, and password hash     |

Both files contain personal or credential data, so they're listed in `.gitignore` and should never be committed.

## Project structure

```text
SmartJatri/
├── main.cpp                   Entry point
├── Utils.h / .cpp             Input helpers, money formatting, string utilities
├── Date.h / .cpp              Calendar dates and day counts
├── User.h / .cpp              User base class and the four card types
├── UserRepository.h / .cpp    Loads and saves cards and trip history
├── AdminAuth.h / .cpp         Admin account and password hashing
└── Kiosk.h / .cpp             Menus and passenger and admin workflows
```

## Design notes

- Each card type is a subclass of `User` that defines its own fare and validity rules. Adding a card type means adding one class.
- Money is stored as integer paisa (1 Tk = 100 paisa), so balances don't suffer from floating-point rounding.
- Expiry uses exact day counts from `Date::toDays()`.
- Cards are owned by `std::unique_ptr`, so memory is freed automatically.

## Known limitations

- The password hash is salted FNV-1a. It's fine for coursework but isn't secure. A real system should use bcrypt, scrypt, or Argon2.
- Stations are free text. There's no route or fare table yet.
- Password masking works on Windows only.
- Only one admin account is supported.
- Deleting the most recent card and then issuing a new one can reuse its UID.

## Acknowledgments

Originally developed for CSE 207 (Data Structures), May 2025.

## License

No license has been chosen yet. Add a `LICENSE` file to set terms for others to use this code.
