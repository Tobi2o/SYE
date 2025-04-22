
## Rapport : Appels systèmes laboratoire 04 Harun Ouweis

### But : Intégration de l'appel système `sys_fork2` dans SO3

#### NB : les instructions que j'ajoute seront proches des instructions fork pour plus de visibilité.

### 1. Attribuer un numéro unique à l'appel système
Chaque appel système est identifié par un numéro unique, assurant que le noyau sait quel traitement effectuer lorsqu'il est appelé.

**Chemin**: `/home/reds/sye23_harun_ouweis/usr/lib/libc/include/syscall.h`  

**Ajouts** :  

#define syscallFork2 240
int sys_fork2(void);


**Explication**:  
Ici, je définis `syscallFork2` avec un numéro unique de 240 et déclare la fonction `sys_fork2` pour une utilisation côté user.

### 2. Établir une convention d'appel entre l'user et le noyau
Cela permet à l'espace user de passer l'exécution au noyau lors de l'appel du système.

**Chemin**: `/home/reds/sye23_harun_ouweis/usr/lib/libc/crt0.S`  

**Ajout** :  

SYSCALLSTUB sys_fork2, syscallFork2


**Explication** :  
Le `SYSCALLSTUB` fait référence à une macro qui génère le code nécessaire pour invoquer l'appel système correspondant. C'est un pont entre l'appel d'une fonction dans l'espace user et l'exécution de l'appel système dans le noyau.

### 3. Définir le numéro d'appel système côté noyau
Il est essentiel que ce numéro corresponde à celui que j'ai choisi plutôt.

**Chemin**: `/home/reds/sye23_harun_ouweis/so3/include/syscall.h`  

**Ajout** :  

#define SYSCALL_FORK2 240


### 4. Ajout du traitement pour `sys_fork2` dans le noyau
J'indique ici comment le noyau doit traiter l'appel système.

**Chemin**: `/home/reds/sye23_harun_ouweis/so3/kernel/syscalls.c`  

**Ajout** :  

case SYSCALL_FORK2:
    result = do_fork2();
    break;


**Explication** :  
Quand l'appel système `sys_fork2` est invoqué, le noyau sait maintenant qu'il doit exécuter la fonction `do_fork2()` pour le traiter.

### 5. Déclaration et implémentation de la fonction de traitement de l'appel système
J'ajoute la logique spécifique pour la création du nouveau processus.

**Chemin**: `/home/reds/sye23_harun_ouweis/so3/include/process.h`  

**Ajout** :  

int do_fork2(void);


**Chemin**: `/home/reds/sye23_harun_ouweis/so3/kernel/process.c`  

**Ajout** :  
- Déclaration de `do_fork2()` 
- Implémentation de la fonction :

  int do_fork2() {
      ...
      printk("fork2() -> process %s with pid %d forked to child with pid %d", parent.name, parent->pid, newp->pid);
      ...
  }


**Explication** :  
La fonction `do_fork2` ressemble en grande partie à la fonction `do_fork` existante mais j'ajoute une instruction `printk` pour afficher le message spécifié lors de la création d'un processus enfant.

---

### Conclusion

L'intégration de l'appel système `sys_fork2` dans SO3 implique un processus en plusieurs étapes qui garantit que l'appel est bien défini tant pour l'espace user que pour l'espace noyau.
