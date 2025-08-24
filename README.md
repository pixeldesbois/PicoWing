# PicoWing

**PicoWing** est un projet open-source d’**avion en papier radiocommandé (RC)**, combinant électronique embarquée, plaisir de voler.  
Ce projet est divisé en 2 parties :
- PicoWingPilot : la télécommande
- PicoWingPlane : la motorisation de l'avion en papier

---

ATTENTION ! Je viens de commencer le projet. Donc rien n'est fonctionnel pour le moment. Je viendrais mettre à jour au fur et à mesure de mes avancées. 

---

## Présentation

L’objectif du projet est de transformer un simple avion en papier en appareil RC complet, grâce à des cartes électroniques ultra-compactes intégrant :

- Un système de commande radio (RC) ESPNOW
- Deux MOSFET de puissance pour la gestion de 2 moteurs
- alimentation et recharge de la batterie lipo
- Une conception permettant aux makers de tous niveaux d'en fabriquer !

---

## PicoWingPilot

C'est la télecommande du projet. Elle permet le passage d'ordres à l'avion via le protocole ESPNOW.
L'ESP32 C3 Pico a été choisit car il intègre la gestion de l'alimentation et la recharge.
Il est en outre très compact.
L'idée est de réaliser une manette proche d'une nunchuk mais il ne possèdera qu'un seul joystick.
Il pourra gérer :
- l'appairage avec l'avion
- le réglage du trim du roulis (droite/gauche)
- le choix de la diffculté de pilotage
- l'affichage d'alertes pour le niveau de batterie de l'avion (via la led RGB de la carte ESP32 C3 Pico)
- la mise en veille de la manette si non utilisée pendant plus de 60s. Un clic sur le joystick le réveillera.

### Matériel côté télécommande
- une carte ESP32 C3 PICO
- un module joystick KY-023
- une lipo 1s

### Firmware pour l'ESP C3 Pico
Le source est dans le dossier `/PicowingPilot/Firmware`

!!!! EN COURS DE DEV !!!!

---

## PicoWingPlane - PARTIE A  VENIR

### Matériel côté avion - PARTIE A  VENIR
- ESP32 C3 PICO
- carte DIY pemettant de gerer l'étage de puissance vers les moteurs
- lipo 1s
- 2 moteurs coreless avec les hélices





---

## Licence

Ce projet est distribué sous licence **MIT** pour le code et **CERN-OHL-S** pour le hardware.


---

## Contributions

Les contributions sont les bienvenues !  
Forkez, ouvrez des issues ou proposez vos améliorations via pull request.

---

**PicoWing — Voler léger, voler libre.**
