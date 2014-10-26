;; Packaging rules for Guix
;; Apparently they don't comply to whatever policy is in use there.
(define-module (gnu packages freedink)
  #:use-module (guix packages)
  #:use-module (guix download)
  #:use-module (guix build-system gnu)
  #:use-module (guix licenses)
  #:use-module (gnu packages gettext)
  #:use-module (gnu packages pkg-config)
  #:use-module (gnu packages sdl)
  #:use-module (gnu packages fontutils)
  #:use-module (gnu packages check)
  #:use-module (gnu packages zip)
)

(define-public freedink-engine
  (package
    (name "freedink-engine")
    (version "108.4")
    (source
     (origin
       (method url-fetch)
       (uri (string-append "mirror://gnu/freedink/freedink-" version ".tar.gz"))
       (sha256 (base32 "08c51imfjfcydm7h0va09z8qfw5nc837bi2x754ni2z737hb5kw2"))))
    (build-system gnu-build-system)
    ;; guix will strip executables, like other distros..
    ;; anyway I'll ditch embedded resources in v109
    (arguments `(#:configure-flags '("--disable-embedded-resources")))
    (native-inputs `(("gettext" ,gnu-gettext)
                     ("pkg-config" ,pkg-config)))
    (inputs `(("sdl" ,sdl)
              ("sdl-image" ,sdl-image)
              ("sdl-mixer" ,sdl-mixer)
              ("sdl-ttf" ,sdl-ttf)
              ("sdl-gfx" ,sdl-gfx)
              ("fontconfig" ,fontconfig)
              ("check" ,check)
	      ))
    (home-page "http://www.gnu.org/software/freedink/")
    (synopsis "humorous top-down adventure and role-playing game (engine)")
    (description "Dink Smallwood is an adventure/role-playing game, similar to Zelda, made by RTsoft. Besides twisted humor, it includes the actual game editor, allowing players to create hundreds of new adventures called Dink Modules or D-Mods for short.

GNU FreeDink is a new and portable version of the game engine, which runs the original game as well as its D-Mods, with close compatibility, under multiple platforms.

This package contains the game engine alone.")
    (license gpl3+)))


(define-public freedink-data
  (package
    (name "freedink-data")
    (version "1.08.20140901")
    (source
     (origin
       (method url-fetch)
       (uri (string-append "mirror://gnu/freedink/freedink-data-" version ".tar.gz"))
       (sha256 (base32 "04f1aa8gfz30qkgv7chjz5n1s8v5hbqs01h2113cq1ylm3isd5sp"))))
    (build-system gnu-build-system)
    (arguments
     `(#:phases (alist-delete 'configure (alist-delete 'check %standard-phases))
       #:make-flags (list (string-append "PREFIX=" (assoc-ref %outputs "out")))))
    (home-page "http://www.gnu.org/software/freedink/")
    (synopsis "humorous top-down adventure and role-playing game (game data)")
    (description "Dink Smallwood is an adventure/role-playing game, similar to Zelda, made by RTsoft. Besides twisted humor, it includes the actual game editor, allowing players to create hundreds of new adventures called Dink Modules or D-Mods for short.

GNU FreeDink is a new and portable version of the game engine, which runs the original game as well as its D-Mods, with close compatibility, under multiple platforms.

This package contains the original game story, along with free sound and music replacements.")
    (license gpl3+)))
;; Note: no idea on how to make freedink-engine detect where that one is installed


;; Not packaging freedink-dfarc since there's no wxWidgets package
