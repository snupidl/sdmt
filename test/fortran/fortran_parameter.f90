!!!
!	Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
!	This work was supported by Next-Generation Information Computing Development
!	Program through the National Research Foundation of Korea(NRF)
!	funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
!	Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
!	Contact: sdmt@kdb.snu.ac.kr
!!!

program fortran_para

use fortsdmt
use iso_c_binding
implicit none
logical(1) :: c_b
character(len=25) :: config1 = "./config_fortran_test.xml"
character(len=26) :: config2 = "./config_fortran_test2.xml"
character(len=9) :: test_para = "test_para"
character(len=32) :: arg
integer :: res, val, i

c_b = .false.

do i = 1, command_argument_count()
	call get_command_argument(i , arg)
	select case (arg)
		case('1')
			res = sdmt_init(config1, c_b)
			res = sdmt_register_int_parameter(test_para, 100)
			val = sdmt_get_int_parameter(test_para)
			if (val .ne. 100) then
				print *, 'error' 
			end if
			res = sdmt_finalize()
			stop
		case('2')
			res = sdmt_init(config2, c_b)
			val = sdmt_get_int_parameter(test_para)
			if (val .ne. 200) then
				print *, 'error' 
			end if
			res = sdmt_finalize()
			stop
	end select
end do
end program
