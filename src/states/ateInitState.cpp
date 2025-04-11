#include "states.hpp"
#include "processor.hpp"


// if (shouldStop) {
//     std::cout << "[ATEInitState] Interrupted.\n";
//     return;
// }

void ATEInitState::transitionT1ToIdle(Processor& processor) {
    std::cout << "[ATEInitState] Transitioning T1." << std::endl;
    processor.changeState(std::make_shared<ATEIdleState>());
}

void ATEInitState::execute(Processor& processor) {
    std::cout << "[ATEInitState] Start of execute.\n";

    // Demarrer serviceExecDialog
    // -> code final = le bon CreateProcess();
    std::cout << "[ATEInitState] Starting serviceExecDialog..." << std::endl;

    auto msg = processor.waitForMessage({1});
    // Check que le msg dit que tout va bien (regarder code extension)  ou est-ce que juste le fait de recevoir le message veut dire que tout va bien ???
    // Pour l'exemple ici on considere que le messgae recu veut dire que tout va bien donc on peut continuer

    // Attendre que le postier du Dialog soit demarré
    msg = processor.waitForMessage({3});


    // Demarrer serviceExecExec
    // -> code final = le bon CreateProcess();
    std::cout << "[ATEInitState] Starting serviceExecExec..." << std::endl;
    msg = processor.waitForMessage({2});
    // Pour l'exemple on dit ici que faut checker le contenu du message
    // Regarder l'extension, si elle dit que c'est KO alors gerer cas d'erreur, sinon on peut continuer


    // Attendre que les 2 postiers soient ready (etat initialisé)

    transitionT1ToIdle(processor);
    return;

    std::cout << "[ATEInitState] execute terminated." << std::endl;

}

// Version attente en // de pluisuers RAEDY
// void ATEInitState::execute(Processor& processor) {
//     std::cout << "[ATEInitState] Start of execute.\n";
//
//     // Demarrer serviceExecDialog
//
//     // Demarrer serviceExecExec
//
//     // Attendre le ready des 2 services
//
//     bool readyBothServiceExec = false;
//     bool readyServiceExecDialog = false;
//     bool readyServiceExecExec = false;
//
//     while (readyBothServiceExec == false) {
//         auto msg = processor.waitForMessage({1,2}); // Accept ids from a set
//         //si msg ready dialog alors readyServiceExecDialog = true
//         // si msg ready exec alors readyServiceExecExec = true
//         // si les 2 sont true alors readyBothServiceExec = true
//     }
//
//     // Demander demarrage Postier Dialogue au serviceExecDialog
//     // Demander demarrage Postier Exec au serviceExecExec
//
//     // Attendre que les 2 postiers soient ready (etat initialisé)
//
//     transitionT1ToIdle(processor);
//     return;
//
//     std::cout << "[ATEInitState] execute terminated." << std::endl;
//
// }
