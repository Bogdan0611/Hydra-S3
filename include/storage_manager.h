#pragma once

// acces la cardul microSD - toate modulele viitoare vor salva date aici
// (capturi RAW, UID-uri citite, etc), centralizez initializarea aici
// ca sa nu o repete fiecare modul separat

class StorageManager {
public:
    // false daca SD nu e gasit (lipsa, prost bagat, sau fire gresite)
    bool begin();

    // verifica daca SD e disponibil - de apelat inainte de orice scriere,
    // ca sa nu incerc sa scriu pe un card care nu exista
    bool isReady() { return ready; }

private:
    bool ready = false;
};
