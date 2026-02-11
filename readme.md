# MGL849 - Laboratoire 1 : Système de Contrôle Environnemental Temps Réel

## Description

Ce projet implémente un système de contrôle environnemental temps réel sur Raspberry Pi utilisant des capteurs I2C et une communication réseau. Le système lit les données de température, pression et humidité, calcule la puissance de chauffage nécessaire et transmet toutes les informations à un serveur distant.

## Matériel Requis

- **Raspberry Pi** avec Linux
- **Sensor hat** avec **Capteur LPS25H** et **Capteur HTS221**
- **Clavier USB** pour contrôle interactif
- Connexion réseau (locale ou distante)

## Fonctionnalités

### Acquisition de Données
- **Température** : Lecture depuis le capteur LPS25H toutes les 2 secondes
- **Pression** : Lecture depuis le capteur LPS25H toutes les 2 secondes
- **Humidité** : Lecture depuis le capteur HTS221 toutes les 2 secondes

### Contrôle
- **Température cible** : Ajustable par clavier (5°C à 50°C)
  - Touche `+` : Augmente de 1°C
  - Touche `-` : Diminue de 1°C
- **Calcul de puissance** : Algorithme proportionnel basé sur l'écart température cible/réelle

### Communication Réseau
- Transmission des données vers un serveur TCP/IP
- Protocole personnalisé pour température, pression, humidité et puissance
- Connexion configurable (IP et port)

### Temps Réel
- **Threads POSIX** avec ordonnancement SCHED_FIFO
- **Priorités** :
  - Priorité haute (90) : Transmission température cible (déclenchée par événement clavier)
  - Priorité normale (50) : Autres tâches
- **Synchronisation** : Mutex et variables conditionnelles

## Architecture du Code

Le code est découpé en deux parties : include représente les fichiers .h ainsi que toutes les déclarations et headers nécessaires au répertoire src qui gère l'implémentation.

- drivers → gère toutes les communications (I2C, clavier)
- network → gère les interactions et échanges réseau avec l'afficheur TCP/IP
- sensors → gère les capteurs sur le sensor hat
- config → contient des déclarations de variables pour éviter les nombres magiques
- shared_data → la structure de données partagée entre les threads
- tasks → découpage des tâches pour être exécutées dans des threads indépendants

# Utilisation

À la racine du projet, il y a un makefile avec la liste des commandes possibles.

## Compilation

```bash
make
```

## Nettoyage
```bash
make clean
```

### Recompilation complète
```bash
make rebuild
```

## Utilisation

### Exécution avec serveur local
```bash
sudo make run
```

### Exécution avec serveur distant
```bash
sudo make run IP=192.168.1.100 PORT=5000
```

**Note** : Les droits `sudo` sont requis pour :
- Utiliser l'ordonnancement temps réel (SCHED_FIFO)


## Arrêt Gracieux

Le programme gère proprement les signaux d'interruption (Ctrl+C) :
- Annulation de tous les threads
- Fermeture des connexions réseau
- Fermeture des bus I2C
- Fermeture du périphérique clavier
- Libération de toutes les ressources

## Auteur

Projet réalisé dans le cadre du cours MGL849 - Systèmes Temps Réel