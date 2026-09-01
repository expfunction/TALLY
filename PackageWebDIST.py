import os, sys, zipfile, shutil, stat

def package_web():
    root_dir = os.path.dirname(os.path.abspath(__file__))
    dist_dir = os.path.join(root_dir, 'DIST')
    assts_dir = os.path.join(root_dir, 'ASSTS')
    bin_exe = os.path.join(root_dir, 'bin', 'CONTD.exe')
    jsdos_path = os.path.join(dist_dir, 'CONTD.jsdos')
    web_zip_path = os.path.join(dist_dir, 'contdweb.zip')

    os.makedirs(dist_dir, exist_ok=True)
    temp_dir = os.path.join(root_dir, 'scratch', 'web_bundle_build')
    if os.path.exists(temp_dir):
        shutil.rmtree(temp_dir)
    os.makedirs(temp_dir, exist_ok=True)

    print('[1/4] Extracting emulator runtime assets...')
    existing_zip = web_zip_path if os.path.exists(web_zip_path) else None
    if existing_zip:
        with zipfile.ZipFile(existing_zip, 'r') as file:
            for item in file.infolist():
                if item.filename != 'index.html' and item.filename != 'CONTD.jsdos':
                    file.extract(item, temp_dir)

    print('[2/4] Updating CONTD.jsdos bundle...')
    dos_temp = os.path.join(temp_dir, 'jsdos_contents')
    os.makedirs(dos_temp, exist_ok=True)
    if os.path.exists(jsdos_path):
        with zipfile.ZipFile(jsdos_path, 'r') as file:
            for item in file.infolist():
                if item.filename.startswith('.jsdos/') or item.filename.startswith('CWSDPM'):
                    file.extract(item, dos_temp)

    # Write optimized dosbox.conf for fast Wasm emulation (fixed cycles & correct IRQ)
    os.makedirs(os.path.join(dos_temp, '.jsdos'), exist_ok=True)
    optimized_dosbox_conf = '''[sdl]
autolock=false
fullscreen=false
fulldouble=false
fullresolution=original
windowresolution=original
output=surface
sensitivity=100
waitonerror=true
priority=higher,normal
mapperfile=mapper-jsdos.map
usescancodes=true
vsync=false

[dosbox]
machine=svga_s3
memsize=32

[cpu]
core=auto
cputype=auto
cycles=max
cycleup=1000
cycledown=1000

[mixer]
nosound=false
rate=22050
blocksize=1024
prebuffer=40

[render]
frameskip=0
aspect=false
scaler=none

[midi]
mpu401=intelligent
mididevice=default

[sblaster]
sbtype=sb16
sbbase=220
irq=5
dma=1
hdma=5
sbmixer=true
oplmode=auto
oplemu=default
oplrate=22050

[gus]
gus=false

[speaker]
pcspeaker=true
pcrate=22050
tandy=auto
tandyrate=22050
disney=true

[joystick]
joysticktype=none

[serial]
serial1=dummy
serial2=dummy
serial3=disabled
serial4=disabled

[dos]
lfn=true
xms=true
ems=true
umb=true
keyboardlayout=auto

[ipx]
ipx=false

[autoexec]
@echo off
mount c .
c:
cls
bin\\CONTD.exe
'''
    with open(os.path.join(dos_temp, '.jsdos', 'dosbox.conf'), 'w', encoding='utf-8') as f:
        f.write(optimized_dosbox_conf)

    cwsdpmi_candidates = [
        os.path.join(dos_temp, 'CWSDPMI.EXE'),
        r'd:\Program Files\Dosbox-X\drivec\DJGPP\bin\CWSDPMI.EXE',
        r'd:\Program Files\Dosbox-X\drivec\NASM\cwsdpmi.exe'
    ]
    cwsdpmi_src = next((p for p in cwsdpmi_candidates if os.path.exists(p)), None)

    with zipfile.ZipFile(jsdos_path, 'w', compression=zipfile.ZIP_DEFLATED) as z:
        added_dirs = set()
        def add_zip_dir(arcname):
            norm = arcname.replace('\\', '/').strip('/') + '/'
            if norm not in added_dirs and norm != '/':
                parts = norm.rstrip('/').split('/')
                cur = ''
                for part in parts:
                    cur += part + '/'
                    if cur not in added_dirs:
                        zi = zipfile.ZipInfo(cur)
                        zi.create_system = 3  # UNIX
                        zi.external_attr = (stat.S_IFDIR | 0o755) << 16
                        z.writestr(zi, b'', compress_type=zipfile.ZIP_STORED)
                        added_dirs.add(cur)

        def add_zip_file(local_path, arcname):
            norm = arcname.replace('\\', '/')
            parent = os.path.dirname(norm)
            if parent:
                add_zip_dir(parent)
            with open(local_path, 'rb') as f:
                data = f.read()
            zi = zipfile.ZipInfo(norm)
            zi.create_system = 3  # UNIX
            zi.external_attr = (stat.S_IFREG | 0o644) << 16
            z.writestr(zi, data, compress_type=zipfile.ZIP_DEFLATED)

        # 1. Add .jsdos directory and config files
        add_zip_dir('.jsdos')
        for root, dirs, files in os.walk(os.path.join(dos_temp, '.jsdos')):
            for d in dirs:
                rel_d = '.jsdos/' + os.path.relpath(os.path.join(root, d), os.path.join(dos_temp, '.jsdos'))
                add_zip_dir(rel_d)
            for f in files:
                full_p = os.path.join(root, f)
                rel_p = '.jsdos/' + os.path.relpath(full_p, os.path.join(dos_temp, '.jsdos')).replace('\\', '/')
                add_zip_file(full_p, rel_p)
        
        # 2. Add CWSDPMI.EXE at root and in bin/
        if cwsdpmi_src:
            add_zip_file(cwsdpmi_src, 'CWSDPMI.EXE')
            add_zip_file(cwsdpmi_src, 'bin/CWSDPMI.EXE')
            print(f'      Included CWSDPMI.EXE from {cwsdpmi_src}')
        
        # 3. Add bin/ and bin/CONTD.exe
        if os.path.exists(bin_exe):
            add_zip_file(bin_exe, 'bin/CONTD.exe')
        
        # 4. Add ASSTS/ and all recursive subdirectories and files
        add_zip_dir('ASSTS')
        written_paths = set()
        for root, dirs, files in os.walk(assts_dir):
            for d in dirs:
                rel_d = os.path.relpath(os.path.join(root, d), root_dir)
                add_zip_dir(rel_d)
            for f in files:
                full_p = os.path.join(root, f)
                rel_p = os.path.relpath(full_p, root_dir).replace('\\', '/')
                if rel_p.lower() not in written_paths:
                    add_zip_file(full_p, rel_p)
                    written_paths.add(rel_p.lower())

    print(f'      Updated {jsdos_path} ({os.path.getsize(jsdos_path):,} bytes)')

    shutil.copy2(jsdos_path, os.path.join(temp_dir, 'CONTD.jsdos'))

    print('[3/4] Generating standalone index.html (offline & itch.io compatible)...')
    html_content = '''<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>COUNTED</title>
  <style>
    html, body { margin: 0; background: #070814; height: 100%; }
    #dos { width: 100%; height: 100%; }
  </style>
  <script>
    // Intercept keyboard lock inside itch.io iframes to prevent InvalidStateError
    (function() {
      try {
        if (typeof navigator !== 'undefined' && navigator.keyboard) {
          Object.defineProperty(navigator.keyboard, 'lock', {
            value: function() { return Promise.resolve(); },
            writable: true,
            configurable: true
          });
        }
      } catch (e) {}
      window.addEventListener('unhandledrejection', function(e) {
        if (e.reason && (e.reason.name === 'InvalidStateError' || (e.reason.message && e.reason.message.includes('lock')))) {
          e.preventDefault();
        }
      });
    })();

    // Resume AudioContext on first user gesture to prevent sampleRate === 0 warnings
    const unlockAudio = () => {
      try {
        const AudioCtx = window.AudioContext || window.webkitAudioContext;
        if (AudioCtx) {
          const ctx = new AudioCtx();
          if (ctx.state === 'suspended') ctx.resume();
        }
      } catch (e) {}
    };
    window.addEventListener('click', unlockAudio, { once: true });
    window.addEventListener('keydown', unlockAudio, { once: true });
  </script>
  <link rel="stylesheet" href="https://v8.js-dos.com/latest/js-dos.css">
  <script src="https://v8.js-dos.com/latest/js-dos.js"></script>
</head>
<body>
  <div id="dos"></div>
  <script>
    Dos(document.getElementById("dos"), {
      url: "CONTD.jsdos",
      pathPrefix: "https://v8.js-dos.com/latest/emulators/",
      theme: "dark",
      autoStart: true,
      kiosk: true,
      imageRendering: "pixelated"
    });
  </script>
</body>
</html>
'''
    with open(os.path.join(temp_dir, 'index.html'), 'w', encoding='utf-8') as f:
        f.write(html_content)

    print('[4/4] Creating web release archives (contdweb.zip / COUNTED_WEB_HTML5.zip)...')
    with zipfile.ZipFile(web_zip_path, 'w', compression=zipfile.ZIP_DEFLATED) as file:
        outer_added_dirs = set()
        def add_outer_dir(arcname):
            norm = arcname.replace('\\', '/').strip('/') + '/'
            if norm not in outer_added_dirs and norm != '/':
                parts = norm.rstrip('/').split('/')
                cur = ''
                for part in parts:
                    cur += part + '/'
                    if cur not in outer_added_dirs:
                        zi = zipfile.ZipInfo(cur)
                        zi.create_system = 3  # UNIX
                        zi.external_attr = (stat.S_IFDIR | 0o755) << 16
                        file.writestr(zi, b'', compress_type=zipfile.ZIP_STORED)
                        outer_added_dirs.add(cur)

        def add_outer_file(local_path, arcname):
            norm = arcname.replace('\\', '/')
            parent = os.path.dirname(norm)
            if parent:
                add_outer_dir(parent)
            with open(local_path, 'rb') as f:
                data = f.read()
            zi = zipfile.ZipInfo(norm)
            zi.create_system = 3  # UNIX
            zi.external_attr = (stat.S_IFREG | 0o644) << 16
            file.writestr(zi, data, compress_type=zipfile.ZIP_DEFLATED)

        add_outer_file(os.path.join(temp_dir, 'index.html'), 'index.html')
        for root, dirs, files in os.walk(temp_dir):
            for d in dirs:
                if 'jsdos_contents' in d:
                    continue
                rel_d = os.path.relpath(os.path.join(root, d), temp_dir)
                if 'jsdos_contents' not in rel_d:
                    add_outer_dir(rel_d)
            for f in files:
                if f == 'index.html' and root == temp_dir:
                    continue
                if 'jsdos_contents' in root:
                    continue
                full_p = os.path.join(root, f)
                rel_p = os.path.relpath(full_p, temp_dir).replace('\\', '/')
                add_outer_file(full_p, rel_p)

    # Also create alias COUNTED_WEB_HTML5.zip for clarity on itch.io uploads
    named_zip_path = os.path.join(dist_dir, 'COUNTED_WEB_HTML5.zip')
    shutil.copy2(web_zip_path, named_zip_path)

    print(f'      Web distribution package created: {web_zip_path} ({os.path.getsize(web_zip_path):,} bytes)')
    print(f'      Itch.io upload bundle ready:      {named_zip_path}')
    
    print('Verifying zip entries:')
    with zipfile.ZipFile(web_zip_path, 'r') as zf:
        entries = zf.namelist()
        print(f'  Total entries: {len(entries)}')
        print(f'  index.html present at root: {"index.html" in entries}')
        print(f'  CONTD.jsdos present: {"CONTD.jsdos" in entries}')
        print(f'  js-dos.js present: {"js-dos.js" in entries}')
        print(f'  js-dos.css present: {"js-dos.css" in entries}')
        print(f'  emulators/wdosbox.wasm present: {"emulators/wdosbox.wasm" in entries}')

if __name__ == '__main__':
    package_web()
