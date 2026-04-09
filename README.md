#  Projet : Du capteur au banc de test - Jauge de contrainte en graphite


L'objectif principal de ce projet est d'évaluer une technologie "low-tech" : un capteur de déformation (jauge de contrainte) fabriqué à partir de graphite. Pour cela, nous avons conçu de A à Z une chaîne de mesure complète, allant de la physique du capteur jusqu'à l'application smartphone, en passant par le conditionnement électronique et la création d'un banc de test automatisé.

##  Équipe Projet
* **[Jules Gleyzes]** * **[Samy Shihadeh]**
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
* **Support :** Papier classique. * **Matériau sensible :** Dépôt de graphite par frottement. * **Dimensions :** Motif en forme de "U" avec une longueur de bande de 35 mm et un espacement de 5 mm. 
### 1.2. Principe Physique : 
Le réseau de percolation et l'Effet Tunnel Contrairement à un fil de cuivre continu, le trait de crayon déposé sur la rugosité du papier forme un **réseau de percolation** : une multitude de micro et nano-paillettes de graphite dispersées de façon chaotique. Pour que le courant traverse le capteur, les électrons utilisent deux mécanismes : 1. **La conduction ohmique :** Lorsque les paillettes se touchent physiquement. 2. **L'effet tunnel quantique :** Lorsque deux paillettes sont séparées par un vide ou un isolant nanométrique, les électrons (qui se comportent comme des ondes) ont une probabilité de "traverser" cette barrière. La résistance électrique liée à cet effet tunnel ($R_{tunnel}$) dépend de manière **exponentielle** de la distance ($d$) entre les paillettes de graphite : $$R_{tunnel} \propto \exp(\gamma d)$$ ### 1.3. Réponse à la déformation (Macroscopique) C'est cette sensibilité exponentielle à la distance nanométrique qui permet au capteur de mesurer une déformation mécanique : * **En Tension (étirement / courbure vers l'extérieur) :** Les fibres du papier s'étirent, éloignant les paillettes de graphite les unes des autres. La distance $d$ des jonctions augmente. L'effet tunnel s'effondre, ce qui **augmente drastiquement la résistance électrique globale**. * **En Compression (courbure vers l'intérieur) :** Les fibres du papier se tassent. Les paillettes sont rapprochées, créant de nouveaux contacts physiques et diminuant la distance $d$ des jonctions tunnel. Les électrons circulent beaucoup plus facilement, ce qui **diminue la résistance électrique**. Ce capteur low-tech est donc, fondamentalement, un détecteur quantique de variations de distances nanométriques.
