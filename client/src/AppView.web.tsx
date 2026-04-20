import AppExperience from "./AppExperience";
import { BoardConnectionProvider } from "./state/BoardConnectionContext";
import { SafeAreaProvider } from "react-native-safe-area-context";

export default function AppViewWeb() {
  return (
    <SafeAreaProvider>
      <BoardConnectionProvider>
        <AppExperience />
      </BoardConnectionProvider>
    </SafeAreaProvider>
  );
}
