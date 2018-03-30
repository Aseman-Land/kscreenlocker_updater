# KScreenLocker.Updater
Update kscreenlocker background based of the Plasma wallpaper. It's just a small binary, written using Qt. It's free, open-source and released under the GPLv3 license.
It's only works for KDE/Plasma users on the Linux (Tested just on the ubuntu/kubuntu)

![](http://aseman.co/github/kscreenlocker_updater/screenlocker_updater_5.jpg)

## How it works?

It watch `$HOME/.config/plasma-org.kde.plasma.desktop-appletsrc` file for changes, If your wallpaper changed, It creates a blurred image from your wallpaper to `$HOME/.cache/kscreenlocker_updater/` and Update the `$HOME/.config/kscreenlockerrc` file to the new lock-screen background.

### Files to watch or changes:

```yaml
Watch:
$HOME/.config/plasma-org.kde.plasma.desktop-appletsrc

Change:
$HOME/.config/kscreenlockerrc

Cache for blurred images:
$HOME/.cache/kscreenlocker_updater/
```

## How to build?

It's easy to build it. first install dependencies

```bash
sudo apt-get install g++ git qt5-default qtbase5-dev
```

Then get source from the git, build and install it:

```bash
git clone https://github.com/Aseman-Land/kscreenlocker_updater.git
cd kscreenlocker_updater
mkdir build && cd build
qmake .. PREFIX=/usr
make
sudo make install
```

## How to run?

Before start it just set your lock-screen mode to Image type from `System Settings > Screen Locking > Appearance > Wallpaper Type`

![](http://aseman.co/github/kscreenlocker_updater/configs.jpg)

And then, Just simply run it:

```bash
kscreenlocker_updater
```

You can also put it to the startup to run it automatically every time you logged-in to your Plasma-Desktop
