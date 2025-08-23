# PicoWing

**PicoWing** est un projet open-source d’**avion en papier radiocommandé (RC)**, combinant électronique embarquée, plaisir de voler.  

---
ATTENTION ! Je viens de commencer le projet. Donc rien n'est fonctionnel pour le moment. Je viendrais mettre à jour au fur et à mesure de mes avancées. 
---

## Présentation

L’objectif du projet est de transformer un simple avion en papier en appareil RC complet,  
grâce à des cartes électroniques ultra-compactes intégrant :

- Un système de commande radio (RC) ESPNOW
- Deux MOSFET de puissance pour la gestion de 2 moteurs
- alimentation et recharge de la batterie lipo
- Une conception permettant aux makers de tous niveaux d'en fabriquer !

---
## PicoWingPilot

C'est la télecommande du projet. Elle communique à l'avion via le protocole ESPNOW.
L'ESP32 C3 Pico a été choisit car il intègre la gestion de l'alimentation et la recharge.

### Matériel côté télécommande
une carte ESP32 C3 PICO
un module joystick KY-023
une lipo 1s

---
## PicoWingPlane

### Matériel côté avion - PARTIE A  VENIR
ESP32 C3 PICO
carte DIY pemettant de gerer l'étage de puissance vers les moteurs
lipo 1s
2 moteurs coreless avec les hélices

*(BOM complet dans le dossier `/hardware`)*



---

## Licence

Ce projet est distribué sous licence **MIT** pour le code et **CERN-OHL-S** pour le hardware.

---

## 📸 Aperçu  - PARTIE A  VENIR



---

## Contributions

Les contributions sont les bienvenues !  
Forkez, ouvrez des issues ou proposez vos améliorations via pull request.

---

**PicoWing — Voler léger, voler libre.**
