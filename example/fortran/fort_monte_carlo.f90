!!!

!	Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
!	This work was supported by Next-Generation Information Computing Development
!	Program through the National Research Foundation of Korea(NRF)
!	funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
!	Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
!	Contact: sdmt@kdb.snu.ac.kr

!	estimating the value of Pi using Monte Carlo method
!	implemented by referring to
!	https://www.geeksforgeeks.org/estimating-value-pi-using-monte-carlo/

!!!

program fort_monte_carlo

use fortsdmt
use iso_c_binding
implicit none
integer :: count, n,i
integer :: res
logical(1) :: c_b
character(len=12) :: config = "./config.xml"
character(len=2) :: pi_name = 'pi'
character(len=6) :: square_name = 'square'
character(len=6) :: circle_name = 'circle'

real(kind=8), pointer :: pi_ptr
integer(kind=8), pointer :: sq_ptr
integer(kind=8), pointer :: ci_ptr

real(kind=8), target :: pi = 0.0
integer(kind=8), target :: sq = 0, ci = 0


real :: r, x, y
count = 0
n = 1000000

c_b = .false.

res = sdmt_init(config, c_b)

res = sdmt_register_snapshot(pi_name, SDMT_DOUBLE, SDMT_SCALAR, 0)
res = sdmt_register_snapshot(square_name, SDMT_INT, SDMT_SCALAR, 0)
res = sdmt_register_snapshot(circle_name, SDMT_INT, SDMT_SCALAR, 0)

call c_f_pointer(sdmt_doubleptr_c(pi_name), pi_ptr)
call c_f_pointer(sdmt_intptr_c(square_name), sq_ptr)
call c_f_pointer(sdmt_intptr_c(circle_name), ci_ptr)

do i = sdmt_iter(), n
	call srand(35791246)
	call random_number(x)
	call random_number(y)
	if (x*x + y*y .le. 1.0) then
		ci = ci + 1
		ci_ptr => ci
	end if
	sq = sq + 1
	sq_ptr => sq
	!pi_ptr = real(4 * ci_ptr) / sq_ptr
	pi = real(4*ci) / sq
	pi_ptr => pi
	if (mod(i , 10000) .eq. 0 .and. i > 0) then
		res = sdmt_checkpoint(1)
		print*, i, 'th iteration has processed, currentPi is ', pi_ptr
	end if
end do


res = sdmt_finalize()

end program
