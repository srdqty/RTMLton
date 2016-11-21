(* at this point, basis is booting, once callFromCHandler
   is set, the following SML code will execute via
   the trampoline in posix thread 0. If you do not
   see the print output, then the trampoline isnt working *)
   open OS.Process
val a = 9320
val _ = print "test2 was running and is going to sleep\n"

fun inf_sleep t= OS.Process.sleep t

fun pause () = (print "sleep\n"; inf_sleep (Time.fromSeconds 20)
                ; pause ())

val _ = pause ()


