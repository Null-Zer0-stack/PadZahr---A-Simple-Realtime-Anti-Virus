#ifndef BLACK_LIST_H
#define BLACK_LIST_H
#include <vector>
#include <string>

// A simple vector of blacklisted process names (case-insensitive for comparison)
const std::vector<std::string> g_blacklistedProcesses = {
    
 
    // I hope all viruses name as same as their executables:XD if not we are doomed:D 
    // Actual list of viruses, malwares, trojans, etc...
    // Adwares List ->
    "BonziBuddy.exe",
    "Gator.exe",
    "Spyware Doctor.exe",
    // Botnet List ->
    "Botnets.exe",
    "Zeus.exe",
    "Zbot.exe",
    "Mirai.exe",
    // Ransomware List ->
    "WannaCry.exe",
    "Petya.exe",
    "Ryuk.exe",
    "Locky.exe",
    "CryptoLocker.exe",
    // Rootkits List ->
    "SubSeven.exe",
    "Fu.exe",
    "Hacker Defender.exe",
    // Spyware List ->
    "FinFisher.exe",
    "FinSpy.exe",
    "CoolWebSearch.exe",
    "Zango.exe",
    // Trojans List ->
    "BackOrifice.exe",
    "NetBus.exe",
    "Zeus.exe",
    "Emotet.exe",
    "TrickBot.exe",
    "NanoCore.exe",
    //Viruses List ->
    "ILoveYou.exe",
    "LoveBug.exe",
    "Melissa.exe",
    "Mydoom.exe",
    "Code Red.exe",
    "Stuxnet.exe",
    "Conficker.exe",
    // Worms List ->
    "Blaster.exe",
    "Lovsan.exe",
    "SQL Slammerexe",
    "Mydoom.exe"
};




#endif 
