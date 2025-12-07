# Configuration de la musique d'ascenseur

## Installation de la musique

Le programme supporte la lecture de musique de fond. Pour l'activer, vous devez :

### 1. Installer un lecteur audio

Le programme essaie d'utiliser `mpg123` (pour MP3) ou `aplay` (pour WAV).

Installation sur Debian/Ubuntu :
```bash
sudo apt-get install mpg123
# ou pour WAV
sudo apt-get install alsa-utils
```

### 2. Ajouter un fichier audio

Placez votre fichier de musique d'ascenseur dans le répertoire du projet avec l'un de ces noms :
- `elevator_music.mp3` (format MP3)
- `elevator_music.wav` (format WAV)

### 3. Fichiers de musique d'ascenseur gratuits

Vous pouvez télécharger de la musique libre de droits sur :
- https://freemusicarchive.org/
- https://incompetech.com/music/
- https://www.bensound.com/

Recherchez des termes comme "elevator music", "lounge", "jazz", "ambient".

## Contrôles de la musique

- **Dans le menu** : Appuyez sur `M` pour activer/désactiver la musique
- **Pendant la simulation** : Appuyez sur `M` pour activer/désactiver la musique
- La musique démarre automatiquement au lancement (si un fichier est trouvé)
- Le statut de la musique s'affiche dans le menu et dans les statistiques

## Dépannage

Si la musique ne joue pas :
1. Vérifiez que `mpg123` ou `aplay` est installé : `which mpg123`
2. Vérifiez que le fichier audio existe : `ls -l elevator_music.*`
3. Testez le fichier manuellement : `mpg123 elevator_music.mp3`
4. Vérifiez que le son n'est pas coupé sur votre système

## Note

La musique joue en boucle indéfiniment jusqu'à ce que vous la désactiviez ou quittiez le programme.
