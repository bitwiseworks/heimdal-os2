; $Id$

(let* ((heimdal-version (getenv "HV"))
       (version-string (concat "Release " heimdal-version)))
  (find-file "configure.in")
  (re-search-forward "AM_INIT_AUTOMAKE(heimdal,\\(.*\\))")
  (replace-match heimdal-version nil nil nil 1)
  (goto-char 1)
  (re-search-forward "AC_INIT(heimdal, *\\([^,]*\\))")
  (replace-match heimdal-version nil nil nil 1)
  (save-buffer)
  ;;(vc-checkin "configure.in" nil version-string)
  (find-file "ChangeLog")
  (add-change-log-entry nil nil nil nil)
  (insert version-string)
  (save-buffer)
  ;;(vc-checkin "ChangeLog" nil version-string)
  (kill-emacs))
