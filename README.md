#  Projet : Du capteur au banc de test - Jauge de contrainte en graphite


L'objectif principal de ce projet est d'évaluer une technologie "low-tech" : un capteur de déformation (jauge de contrainte) fabriqué à partir de graphite. Pour cela, nous avons conçu de A à Z une chaîne de mesure complète, allant de la physique du capteur jusqu'à l'application smartphone, en passant par le conditionnement électronique et la création d'un banc de test automatisé.

##  Équipe Projet
* **[Jules Gleyzes]**  **[Samy Shihadeh]**
* Année : 2025-2026

---

##  Sommaire des livrables
1. [Le Capteur en Graphite](#1-le-capteur-en-graphite-low-tech)
2. [Électronique Analogique et Simulation (LTspice)](#2-électronique-analogique-et-simulation-ltspice)
3. [Le Shield PCB (KiCad)](#3-le-shield-pcb-kicad)
4. [Le Code Arduino](#4-programmation-arduino-et-microcontrôleur)
5. [L'Application Android](#5-application-android-mit-app-inventor)
6. [Le Banc de Test](#6-le-banc-de-test)
7. [La Datasheet Finale](#7-datasheet-du-capteur)

---

## 1. Le Capteur en Graphite (Low-Tech)
Notre jauge de contrainte a été fabriquée selon une approche "low-tech", en crayonnant simplement une zone spécifique sur une feuille de papier avec un crayon à papier (graphite). Derrière cette apparente simplicité se cache en réalité un capteur régi par la mécanique quantique. 
### 1.1. Géométrie et Fabrication 
**Support :** Papier classique.  **Matériau sensible :** Dépôt de graphite par frottement.  **Dimensions :** Motif en forme de "U" avec une longueur de bande de 35 mm et un espacement de 5 mm.

<img width="555" height="195" alt="image" src="https://github.com/user-attachments/assets/4cfade2f-00c0-493a-8d01-a379f9a5b628" />




### 1.2. Principe Physique : 
Le réseau de percolation et l'Effet Tunnel Contrairement à un fil de cuivre continu, le trait de crayon déposé sur la rugosité du papier forme un **réseau de percolation** : une multitude de micro et nano-paillettes de graphite dispersées de façon chaotique. Pour que le courant traverse le capteur, les électrons utilisent deux mécanismes : 1. **La conduction ohmique :** Lorsque les paillettes se touchent physiquement. 2. **L'effet tunnel quantique :** Lorsque deux paillettes sont séparées par un vide ou un isolant nanométrique, les électrons (qui se comportent comme des ondes) ont une probabilité de "traverser" cette barrière. La résistance électrique liée à cet effet tunnel ($R_{tunnel}$) dépend de manière **exponentielle** de la distance ($d$) entre les paillettes de graphite : $$R_{tunnel} \propto \exp(\gamma d)$$ 
### 1.3. Réponse à la déformation (Macroscopique) 
C'est cette sensibilité exponentielle à la distance nanométrique qui permet au capteur de mesurer une déformation mécanique. **En Tension (étirement / courbure vers l'extérieur) :** Les fibres du papier s'étirent, éloignant les paillettes de graphite les unes des autres. La distance $d$ des jonctions augmente. L'effet tunnel s'effondre, ce qui **augmente drastiquement la résistance électrique globale**. **En Compression (courbure vers l'intérieur) :** Les fibres du papier se tassent. Les paillettes sont rapprochées, créant de nouveaux contacts physiques et diminuant la distance $d$ des jonctions tunnel. Les électrons circulent beaucoup plus facilement, ce qui **diminue la résistance électrique**. Ce capteur low-tech est donc, fondamentalement, un détecteur quantique de variations de distances nanométriques.
## 2. Électronique Analogique et Simulation (LTspice)

Avant de passer à la réalisation physique sur notre shield, il est crucial de valider le comportement de notre conditionnement de signal virtuellement. Nous avons utilisé le logiciel LTspice IV pour modéliser le circuit complet afin d'ajuster la valeur de nos composants et vérifier la réponse du système. Notre capteur en graphite possède une résistance électrique très élevée, il ne laisse donc passer qu'un courant de l'ordre du microampère. La carte Arduino ne lisant que des tensions (de 0 à 5V), une interface de traduction est indispensable.

### 2.1. Amplification Transimpédance et Étalonnage Dynamique

Le cœur de notre circuit repose sur un amplificateur opérationnel monté en transimpédance. Ce montage force le très faible courant du capteur à traverser la résistance de rétroaction R4 (100 kΩ) pour générer une tension de sortie que l’arduino est capable de lire. 

### 2.2. Filtrage des Parasites Ambiants

Le graphite et les fils de connexion apportent des signaux parasites à cause des ondes ambiantes. Pour obtenir une mesure propre et exploitable, nous avons étagé deux filtres. 

Le premier est un filtre actif formé par le condensateur C2 (1 µF) placé en parallèle de R4. Il crée un filtre passe-bas du premier ordre dont la fréquence de coupure est calculée par la formule $f_c = \frac{1}{2\pi R_4 C_2}$, ce qui donne environ 1.6 Hz. Cela bloque net le bruit rapide à 50 Hz tout en laissant passer les variations lentes correspondant à nos flexions mécaniques. Enfin, un second filtre passe-bas, passif cette fois, est placé juste avant la broche de lecture de l'Arduino. Composé de R3 (1 kΩ) et C3 (100 nF), il possède une fréquence de coupure d'environ 1.6 kHz pour supprimer les bruits à très haute fréquence et protéger l'entrée du microcontrôleur en agissant comme un filtre anti-repliement.

![Schéma du circuit sur LTspice](Lien_vers_capture_schema_LTspice.png) *<img width="1896" height="904" alt="LTspice" src="https://github.com/user-attachments/assets/29f69778-2f79-4e79-a13f-f97231b6c7ea" />*

## 3. Le Shield PCB (KiCad)
 Pour intégrer proprement notre circuit au microcontrôleur, nous avons conçu un "Shield" sur mesure à l'aide du logiciel **KiCad**. Le principe d'un shield est d'être une carte d'extension conçue pour se connecter directement par-dessus la carte Arduino UNO. Cette approche permet de supprimer le câblage pour avoir un tout plus compact et un visuel moins encombré. 
### Les Composants Intégrés 
Notre carte comprend un ensemble d’éléments matériels nécessaires au fonctionnement du système : 
* **Le capteur en graphite **
* **Le circuit d'amplification transimpédance :** Converti le micro-courant du capteur en une tension (0-5V) lisible par la carte arduino et de filtre les bruits parasites. 
* **Le potentiomètre :** Intégré au circuit d'amplification et piloté par la carte, il sert à étalonner le signal. Il adapte le gain en temps réel pour garantir une lecture précise, peu importe la résistance du capteur fabriqué.
* **L'écran OLED :** L'interface visuelle qui permet de lire les mesures en direct et de visualiser les différents menus de la carte. 
* **L'encodeur rotatif :** Molette cliquable permettant à l'utilisateur de naviguer dans les menus de l'écran et de déclencher les mesures. 
* **Le module Bluetooth :** Transmet les données traitées en temps réel vers notre application smartphone Android. 

<img src="https://github.com/user-attachments/assets/8d7db554-eacc-4a76-acb5-5f6b76f25cb0" width="600">

### De la Conception à la Réalisation 
Pour aboutir à notre carte physique, nous sommes passés par deux grandes étapes de conception sur KiCad : 
**1. Le Schématique Électrique** C'est ici que nous avons défini les liens logiques entre tous nos composants et le microcontrôleur, en utilisant des labels pour garder un document lisible.

<img width="1326" height="877" alt="Schematique" src="https://github.com/user-attachments/assets/fd96e312-4a96-4b44-8af5-b1058f227d68" />

**2. Le Routage (PCB)** Une fois le schéma validé, nous avons placé physiquement les composants sur une carte et tracé les pistes de cuivre (en respectant une largeur et une isolation de 0.5 mm minimum).

<img width="1169" height="846" alt="PCB" src="https://github.com/user-attachments/assets/4fa998a5-6e25-4d96-8b2a-38bfa6b0495d" />
