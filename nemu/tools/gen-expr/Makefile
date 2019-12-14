APP=gen-expr

$(APP): gen-expr.c
	gcc -O2 -Wall -o $@ $<

.PHONY: clean
clean:
	-rm $(APP) .code.c .expr
