program main
  integer a, b, i
  dimension a(3), b(3)
  do i=1,3
     a(i)=i
  end do
  b = a
  do i=1,3
     print *,b(i)
  end do
end program main
