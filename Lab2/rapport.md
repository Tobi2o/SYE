# Rapport Détaillé 

## **Question 1: Exécution de "time_loop" en Mode Continu et en Arrière-plan**

### **Scénario**
- **Commande Exécutée**: `time_loop &` dans le shell.
- **Processus Impliqués**:
  - **Shell (pid 1)**: Passe de *new* → *ready* → *running*.
  - **Time_loop (pid 2)**: Créé et passe de *new* → *ready*.
  - **Autres Commandes (e.g., `ls`)**: Crée un nouveau processus (pid 3), mais reste en attente.

### **Analyse des États de Processus**
- Au démarrage de SO3, le shell s'initialise et devient disponible pour les commandes.
- L'exécution de "time_loop" en arrière-plan crée un nouveau processus qui entre dans l'état *ready* sans interrompre le shell.
- Tentatives d'exécution de nouvelles commandes dans le shell aboutissent à la création de processus supplémentaires qui sont mis en attente car "time_loop" occupe continuellement le CPU.
- Le processus "time_loop" reste en exécution, entrant éventuellement dans l'état *zombie* une fois terminé, permettant alors à d'autres processus en attente d'être exécutés.

### **Conclusion**
- L'exécution en continu de "time_loop" monopolise les ressources CPU, limitant la capacité du shell à gérer d'autres commandes en temps réel.

---

## **Question 2: Modification de la Priorité du Shell**

### **Procédure et Impact**
- **Modification de Priorité**: Exécution de `renice 1 2` pour baisser la priorité du shell.
- **Impact**: Le shell devient incapable d'exécuter de nouvelles commandes pendant l'exécution de "time_loop".

### **Analyse**
- Le changement de priorité place le shell (pid 1) à une priorité inférieure par rapport à "time_loop" (pid 2).
- Ce déséquilibre dans la priorité entraîne une incapacité pour le shell de reprendre le contrôle tant que "time_loop" est en exécution.
- Une fois "time_loop" terminé et passé en état *zombie*, le shell peut reprendre son exécution, traitant les commandes mises en file d'attente.

---

## **Question 3: "time_loop" en Mode Bloquant**

### **Comparaison des Modes**
- **Mode Continu**: Aucune interruption dans l'exécution de "time_loop", le shell reste inactif.
- **Mode Bloquant**: "time_loop" libère périodiquement le CPU pendant les attentes (`usleep()`), permettant l'exécution de commandes dans le shell.

### **Analyse des Comportements**
- En mode bloquant, "time_loop" alterne entre les états *running* et *waiting*, libérant des fenêtres d'exécution pour le shell.
- Cela contraste avec le mode continu où "time_loop" occupe constamment le CPU, empêchant l'exécution d'autres processus.
- L'utilisation de "time_loop" en mode bloquant en arrière-plan améliore la réactivité du shell et la gestion globale des processus.

### **Conclusion**
L'étude des différents scénarios d'ordonnancement par priorité statique dans SO3 révèle l'importance cruciale des paramètres de priorité et des comportements des processus (continu vs bloquant) dans la gestion efficace des ressources système. Cela souligne la nécessité d'une planification et d'une gestion prudentes des processus dans les systèmes d'exploitation pour maintenir un équilibre optimal entre la réactivité et l'utilisation des ressources.
