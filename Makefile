CXXFLAGS=-std=c++17

src = \
	cli/main.cpp \
	interface/console.cpp \
	interface/game_log.cpp \
	interface/game_screens.cpp \
	interface/help_screens.cpp \
	interface/names.cpp \
	interface/questions.cpp \
	interface/screen.cpp \
	interface/setup_screen.cpp \
	interface/text/format.cpp \
	logic/game.cpp \
	logic/player.cpp \
	logic/role.cpp \
	logic/role_ref.cpp \
	logic/rulebook.cpp \
	logic/wildcard.cpp \

objs = $(addprefix build/,$(src:.cpp=.o))


mafia: $(objs)
	$(CXX) $(CXXFLAGS) $(objs) -o mafia

$(objs): build/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $(CXXFLAGS) $< -o $@


clean:
	rm -r ./build
	rm ./mafia


.PHONY: clean
